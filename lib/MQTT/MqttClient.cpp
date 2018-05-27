#include <stddef.h>
#include <MqttClient.h>
#include <TokenIterator.h>
#include <UrlTokenBindings.h>
#include <IntParsing.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <MiLightRadioConfig.h>

using namespace std::placeholders;

MqttClient::MqttClient(Settings& settings, MiLightClient*& milightClient)
  : milightClient(milightClient),
    settings(settings),
    lastConnectAttempt(0)
{
  String strDomain = settings.mqttServer();
  this->domain = new char[strDomain.length() + 1];
  strcpy(this->domain, strDomain.c_str());

}

MqttClient::~MqttClient() {
  mqttClient.disconnect();
  delete this->domain;
}

void MqttClient::begin() {
  char hexVal[2];
  uint8_t fingerprint[SHA1_SIZE];
#ifdef MQTT_DEBUG
  printf_P(
    PSTR("MqttClient - Connecting to: %s\nparsed:%s:%u\n"),
    settings._mqttServer.c_str(),
    settings.mqttServer().c_str(),
    settings.mqttPort()
  );
#endif
  mqttmsg = false;
  mqttClient.setServer(this->domain, settings.mqttPort());
  mqttClient.onMessage(std::bind(&MqttClient::publishCallback, this, _1, _2, _3, _4, _5, _6));
  mqttClient.onConnect(std::bind(&MqttClient::connectCallback, this, _1));
#if ASYNC_TCP_SSL_ENABLED
  mqttClient.setSecure(settings.mqttSecure);
  if(settings.mqttSecure && settings.mqttServerFingerprint.length() == (SHA1_SIZE * 2))
  {
    // Convert string to uint8_t[]
    for(size_t i=0;i<settings.mqttServerFingerprint.length();i+=2)
    {
      hexVal[0] = settings.mqttServerFingerprint[i];
      hexVal[1] = settings.mqttServerFingerprint[i+1];
      fingerprint[i/2]=strtol(hexVal,NULL,16);
    }
    mqttClient.addServerFingerprint(fingerprint);
  }
#endif
  if (settings.mqttUsername.length() > 0) {
    mqttClient.setCredentials(settings.mqttUsername.c_str(), settings.mqttPassword.c_str());
  }

  reconnect();
}

void MqttClient::connect() {
  char nameBuffer[30];
  sprintf_P(nameBuffer, PSTR("milight-hub-%u"), ESP.getChipId());

#ifdef MQTT_DEBUG
    Serial.println(F("MqttClient - connecting"));
#endif

  mqttClient.connect();

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
void MqttClient::connectCallback(bool sessionPresent)
{
  subscribe();

#ifdef MQTT_DEBUG
  Serial.println(F("MqttClient - Successfully connected to MQTT server"));
#endif
}
void MqttClient::handleClient() {
  reconnect();
  if(mqttmsg) publishLoop();
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
  topic.replace(":group_id", "+");
  topic.replace(":device_type", "+");

#ifdef MQTT_DEBUG
  printf_P(PSTR("MqttClient - subscribing to topic: %s\n"), topic.c_str());
#endif

  mqttClient.subscribe(topic.c_str(), 0);
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

  String topic = _topic;
  MqttClient::bindTopicString(topic, remoteConfig, deviceId, groupId);

#ifdef MQTT_DEBUG
  printf("MqttClient - publishing update to %s\n", topic.c_str());
#endif

  mqttClient.publish(topic.c_str(), 0, false, message, retain);
}

void MqttClient::publishCallback(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t  length, size_t index, size_t total) {
  if(!mqttmsg) // For now irgnore new message while process last message.
  {
    strncpy(mqtttopic,topic,strlen(topic));
    mqtttopic[strlen(topic)] = '\0';
    strncpy(mqttpayload,payload,length);
    mqttpayload[length] = '\0';

    mqttmsg = true;
  }
}

void MqttClient::publishLoop(){
  uint16_t deviceId = 0;
  uint8_t groupId = 0;
  const MiLightRemoteConfig* config = &FUT092Config;
  size_t length = strlen(mqttpayload);
  char cstrPayload[length + 1];

  strncpy(cstrPayload, mqttpayload, sizeof cstrPayload - 1);
  cstrPayload[length] = '\0';

#ifdef MQTT_DEBUG
  printf("MqttClient - Got message on topic: %s, %s\n", mqtttopic, mqttpayload);
#endif

  char topicPattern[settings.mqttTopicPattern.length()];
  strcpy(topicPattern, settings.mqttTopicPattern.c_str());

  TokenIterator patternIterator(topicPattern, settings.mqttTopicPattern.length(), '/');
  TokenIterator topicIterator(mqtttopic, strlen(mqtttopic), '/');
  UrlTokenBindings tokenBindings(patternIterator, topicIterator);

  if (tokenBindings.hasBinding("device_id")) {
    deviceId = parseInt<uint16_t>(tokenBindings.get("device_id"));
  }

  if (tokenBindings.hasBinding("group_id")) {
    groupId = parseInt<uint16_t>(tokenBindings.get("group_id"));
  }

  if (tokenBindings.hasBinding("device_type")) {
    config = MiLightRemoteConfig::fromType(tokenBindings.get("device_type"));

    if (config == NULL) {
      Serial.println(F("MqttClient - ERROR: could not extract device_type from topic"));
      return;
    }
  } else {
    Serial.println(F("MqttClient - WARNING: could not find device_type token.  Defaulting to FUT092.\n"));
  }

  StaticJsonBuffer<MQTTBUFFER> buffer;
  JsonObject& obj = buffer.parseObject(cstrPayload);

#ifdef MQTT_DEBUG
  printf("MqttClient - device %04X, group %u\n", deviceId, groupId);
#endif

  milightClient->prepare(config, deviceId, groupId);
  milightClient->update(obj);

  mqttmsg = false;
}

inline void MqttClient::bindTopicString(
  String& topicPattern,
  const MiLightRemoteConfig& remoteConfig,
  const uint16_t deviceId,
  const uint16_t groupId
) {
  String deviceIdHex = String(deviceId, 16);
  deviceIdHex.toUpperCase();
  deviceIdHex = String("0x") + deviceIdHex;

  topicPattern.replace(":device_id", deviceIdHex);
  topicPattern.replace(":hex_device_id", deviceIdHex);
  topicPattern.replace(":dec_device_id", String(deviceId));
  topicPattern.replace(":group_id", String(groupId));
  topicPattern.replace(":device_type", remoteConfig.name);
}
