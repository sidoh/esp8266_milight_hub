#include <stddef.h>
#include <MqttClient.h>
#include <TokenIterator.h>
#include <UrlTokenBindings.h>
#include <IntParsing.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <MiLightRadioConfig.h>
#include <AboutHelper.h>
#include <memory>

#ifdef ESP32
#include <AsyncTCP.h>
#include <freertos/semphr.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#else
#error Platform not supported
#endif

#if ASYNC_TCP_SSL_ENABLED
#include <tcp_axtls.h>
#define SHA1_SIZE 20
#endif

static const char* STATUS_CONNECTED = "connected";
static const char* STATUS_DISCONNECTED = "disconnected_clean";
static const char* STATUS_LWT_DISCONNECTED = "disconnected_unclean";

static const uint8_t MQTT_DEFAULT_QOS = 1;

MqttClient::MqttClient(Settings& settings, MiLightClient*& milightClient)
  : milightClient(milightClient),
    settings(settings),
    lastConnectAttempt(0)
{
  String strDomain = settings.mqttServer();
  this->domain = std::shared_ptr<char>(new char[strDomain.length() + 1], std::default_delete<char[]>());
  strcpy(this->domain.get(), strDomain.c_str());
}

MqttClient::~MqttClient() {
  if (settings.mqttClientStatusTopic.length() > 0) {
    String aboutStr = generateConnectionStatusMessage(STATUS_DISCONNECTED);
    mqttClient.publish(settings.mqttClientStatusTopic.c_str(), MQTT_DEFAULT_QOS, true, aboutStr.c_str());
  }
  mqttClient.disconnect(true);
}

void MqttClient::onConnect(OnConnectFn fn) {
  this->onConnectFn = fn;
}

void MqttClient::begin() {
#ifdef MQTT_DEBUG
  printf_P(
    PSTR("MqttClient - Connecting to: %s\nparsed:%s:%u\n"),
    settings._mqttServer.c_str(),
    settings.mqttServer().c_str(),
    settings.mqttPort()
  );
#endif
  char nameBuffer[30];
  sprintf_P(nameBuffer, PSTR("milight-hub-%u"), ESP.getChipId());

  mqttClient
    .setServer(this->domain, settings.mqttPort())
    .setClientId(nameBuffer);

  if (settings.mqttUsername.length() > 0) {
    mqttClient.setCredentials(settings.mqttUsername.c_str(), settings.mqttPassword.c_str());
  }

  if (settings.mqttClientStatusTopic.length() > 0) {
    // Does not copy the buffer, so we'll have to create space ourselves.
    String lwtMessage = generateConnectionStatusMessage(STATUS_LWT_DISCONNECTED);
    this->lwtBuffer = std::shared_ptr<char>(new char[lwtMessage.length()+1], std::default_delete<char[]>());
    strcpy(this->lwtBuffer.get(), lwtMessage.c_str());

    mqttClient.setWill(
      settings.mqttClientStatusTopic.c_str(),
      MQTT_DEFAULT_QOS,
      true,
      this->lwtBuffer.get()
    );
  }

  mqttClient.onMessage(
    [this](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
      this->publishCallback(topic, payload, len);
    }
  );

  mqttClient.onConnect(
    [this](bool sessionPresent) {
#ifdef MQTT_DEBUG
      Serial.println(F("MqttClient - Successfully connected to MQTT server"));
#endif
      subscribe();
      sendBirthMessage();

      if (this->onConnectFn) {
        this->onConnectFn();
      }
    }
  );

  mqttClient.onDisconnect(
    [this](AsyncMqttClientDisconnectReason disconnectReason) {
#ifdef MQTT_DEBUG
      Serial.printf_P(PSTR("MqttClient - Disconnected: %d\n"), disconnectReason);
#endif
      reconnect();
    }
  );

  reconnect();
}

void MqttClient::connect() {
#ifdef MQTT_DEBUG
    Serial.println(F("MqttClient - connecting"));
#endif
  mqttClient.connect();
}

void MqttClient::sendBirthMessage() {
  if (settings.mqttClientStatusTopic.length() > 0) {
    String aboutStr = generateConnectionStatusMessage(STATUS_CONNECTED);
    mqttClient.publish(settings.mqttClientStatusTopic.c_str(), MQTT_DEFAULT_QOS, true, aboutStr.c_str());
  }
}

void MqttClient::reconnect() {
  if (lastConnectAttempt > 0 && (millis() - lastConnectAttempt) < MQTT_CONNECTION_ATTEMPT_FREQUENCY) {
    return;
  }

  if (! mqttClient.connected()) {
    connect();
  }

  lastConnectAttempt = millis();
}

void MqttClient::sendUpdate(const MiLightRemoteConfig& remoteConfig, uint16_t deviceId, uint16_t groupId, const char* update) {
  publish(settings.mqttUpdateTopicPattern, remoteConfig, deviceId, groupId, update);
}

void MqttClient::sendState(const MiLightRemoteConfig& remoteConfig, uint16_t deviceId, uint16_t groupId, const char* update) {
  publish(settings.mqttStateTopicPattern, remoteConfig, deviceId, groupId, update, true);
}

void MqttClient::subscribe() {
  String topic = settings.mqttTopicPattern;

  topic.replace(":device_id", "+");
  topic.replace(":hex_device_id", "+");
  topic.replace(":dec_device_id", "+");
  topic.replace(":group_id", "+");
  topic.replace(":device_type", "+");
  topic.replace(":device_alias", "+");

#ifdef MQTT_DEBUG
  printf_P(PSTR("MqttClient - subscribing to topic: %s\n"), topic.c_str());
#endif

  mqttClient.subscribe(topic.c_str(), MQTT_DEFAULT_QOS);
}

void MqttClient::send(const char* topic, const char* message, const bool retain) {
  mqttClient.publish(topic, MQTT_DEFAULT_QOS, retain, message);
}

void MqttClient::publish(
  const String& _topic,
  const MiLightRemoteConfig &remoteConfig,
  uint16_t deviceId,
  uint16_t groupId,
  const char* message,
  const bool retain
) {
  if (_topic.length() == 0) {
    return;
  }

  BulbId bulbId(deviceId, groupId, remoteConfig.type);
  String topic = bindTopicString(_topic, bulbId);

#ifdef MQTT_DEBUG
  printf("MqttClient - publishing update to %s\n", topic.c_str());
#endif

  send(topic.c_str(), message, retain);
}

bool MqttClient::isConnected() {
  return mqttClient.connected();
}

void MqttClient::publishCallback(char* topic, char* payload, size_t length) {
  uint16_t deviceId = 0;
  uint8_t groupId = 0;
  const MiLightRemoteConfig* config = &FUT092Config;
  char cstrPayload[length + 1];
  cstrPayload[length] = 0;
  memcpy(cstrPayload, payload, sizeof(byte)*length);

#ifdef MQTT_DEBUG
  printf("MqttClient - Got message on topic: %s\n%s\n", topic, cstrPayload);
#endif

  char topicPattern[settings.mqttTopicPattern.length()];
  strcpy(topicPattern, settings.mqttTopicPattern.c_str());

  TokenIterator patternIterator(topicPattern, settings.mqttTopicPattern.length(), '/');
  TokenIterator topicIterator(topic, strlen(topic), '/');
  UrlTokenBindings tokenBindings(patternIterator, topicIterator);

  if (tokenBindings.hasBinding("device_alias")) {
    String alias = tokenBindings.get("device_alias");
    auto itr = settings.groupIdAliases.find(alias);

    if (itr == settings.groupIdAliases.end()) {
      Serial.printf_P(PSTR("MqttClient - WARNING: could not find device alias: `%s'. Ignoring packet.\n"), alias.c_str());
      return;
    } else {
      BulbId bulbId = itr->second;

      deviceId = bulbId.deviceId;
      config = MiLightRemoteConfig::fromType(bulbId.deviceType);
      groupId = bulbId.groupId;
    }
  } else {
    if (tokenBindings.hasBinding("device_id")) {
      deviceId = parseInt<uint16_t>(tokenBindings.get("device_id"));
    } else if (tokenBindings.hasBinding("hex_device_id")) {
      deviceId = parseInt<uint16_t>(tokenBindings.get("hex_device_id"));
    } else if (tokenBindings.hasBinding("dec_device_id")) {
      deviceId = parseInt<uint16_t>(tokenBindings.get("dec_device_id"));
    }

    if (tokenBindings.hasBinding("group_id")) {
      groupId = parseInt<uint16_t>(tokenBindings.get("group_id"));
    }

    if (tokenBindings.hasBinding("device_type")) {
      config = MiLightRemoteConfig::fromType(tokenBindings.get("device_type"));
    } else {
      Serial.println(F("MqttClient - WARNING: could not find device_type token.  Defaulting to FUT092.\n"));
    }
  }

  if (config == NULL) {
    Serial.println(F("MqttClient - ERROR: unknown device_type specified"));
    return;
  }

  StaticJsonDocument<400> buffer;
  deserializeJson(buffer, cstrPayload);
  JsonObject obj = buffer.as<JsonObject>();

#ifdef MQTT_DEBUG
  printf("MqttClient - device %04X, group %u\n", deviceId, groupId);
#endif

  milightClient->prepare(config, deviceId, groupId);
  milightClient->update(obj);
}

String MqttClient::bindTopicString(const String& topicPattern, const BulbId& bulbId) {
  String boundTopic = topicPattern;
  String deviceIdHex = bulbId.getHexDeviceId();

  boundTopic.replace(":device_id", deviceIdHex);
  boundTopic.replace(":hex_device_id", deviceIdHex);
  boundTopic.replace(":dec_device_id", String(bulbId.deviceId));
  boundTopic.replace(":group_id", String(bulbId.groupId));
  boundTopic.replace(":device_type", MiLightRemoteTypeHelpers::remoteTypeToString(bulbId.deviceType));

  auto it = settings.findAlias(bulbId.deviceType, bulbId.deviceId, bulbId.groupId);
  if (it != settings.groupIdAliases.end()) {
    boundTopic.replace(":device_alias", it->first);
  } else {
    boundTopic.replace(":device_alias", "__unnamed_group");
  }

  return boundTopic;
}

String MqttClient::generateConnectionStatusMessage(const char* connectionStatus) {
  if (settings.simpleMqttClientStatus) {
    // Don't expand disconnect type for simple status
    if (0 == strcmp(connectionStatus, STATUS_CONNECTED)) {
      return connectionStatus;
    } else {
      return "disconnected";
    }
  } else {
    StaticJsonDocument<1024> json;
    json["status"] = connectionStatus;

    // Fill other fields
    AboutHelper::generateAboutObject(json, true);

    String response;
    serializeJson(json, response);

    return response;
  }
}