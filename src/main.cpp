#include <SPI.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <FS.h>
#include <IntParsing.h>
#include <Size.h>
#include <LinkedList.h>
#include <GroupStateStore.h>
#include <MiLightRadioConfig.h>
#include <MiLightRemoteConfig.h>
#include <MiLightHttpServer.h>
#include <Settings.h>
#include <MiLightUdpServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266SSDP.h>
#include <MqttClient.h>
#include <RGBConverter.h>
#include <MiLightDiscoveryServer.h>
#include <MiLightClient.h>
#include <BulbStateUpdater.h>

WiFiManager wifiManager;

Settings settings;

MiLightClient* milightClient = NULL;
MiLightRadioFactory* radioFactory = NULL;
MiLightHttpServer *httpServer = NULL;
MqttClient* mqttClient = NULL;
MiLightDiscoveryServer* discoveryServer = NULL;
uint8_t currentRadioType = 0;

// For tracking and managing group state
GroupStateStore* stateStore = NULL;
BulbStateUpdater* bulbStateUpdater = NULL;

int numUdpServers = 0;
MiLightUdpServer** udpServers = NULL;
WiFiUDP udpSeder;

/**
 * Set up UDP servers (both v5 and v6).  Clean up old ones if necessary.
 */
void initMilightUdpServers() {
  if (udpServers) {
    for (int i = 0; i < numUdpServers; i++) {
      if (udpServers[i]) {
        delete udpServers[i];
      }
    }

    delete udpServers;
  }

  udpServers = new MiLightUdpServer*[settings.numGatewayConfigs];
  numUdpServers = settings.numGatewayConfigs;

  for (size_t i = 0; i < settings.numGatewayConfigs; i++) {
    GatewayConfig* config = settings.gatewayConfigs[i];
    MiLightUdpServer* server = MiLightUdpServer::fromVersion(
      config->protocolVersion,
      milightClient,
      config->port,
      config->deviceId
    );

    if (server == NULL) {
      Serial.print(F("Error creating UDP server with protocol version: "));
      Serial.println(config->protocolVersion);
    } else {
      udpServers[i] = server;
      udpServers[i]->begin();
    }
  }
}

/**
 * Milight RF packet handler.
 *
 * Called both when a packet is sent locally, and when an intercepted packet
 * is read.
 */
void onPacketSentHandler(uint8_t* packet, const MiLightRemoteConfig& config) {
  StaticJsonBuffer<200> buffer;
  JsonObject& result = buffer.createObject();
  BulbId bulbId = config.packetFormatter->parsePacket(packet, result, stateStore);

  if (&bulbId == &DEFAULT_BULB_ID) {
    Serial.println(F("Skipping packet handler because packet was not decoded"));
    return;
  }

  const MiLightRemoteConfig& remoteConfig =
    *MiLightRemoteConfig::fromType(bulbId.deviceType);

  GroupState& groupState = stateStore->get(bulbId);
  groupState.patch(result);
  stateStore->set(bulbId, groupState);

  if (mqttClient) {
    // Sends the state delta derived from the raw packet
    char output[200];
    result.printTo(output);
    mqttClient->sendUpdate(remoteConfig, bulbId.deviceId, bulbId.groupId, output);

    // Sends the entire state
    bulbStateUpdater->enqueueUpdate(bulbId, groupState);
  }

  httpServer->handlePacketSent(packet, remoteConfig);
}

/**
 * Listen for packets on one radio config.  Cycles through all configs as its
 * called.
 */
void handleListen() {
  if (! settings.listenRepeats) {
    return;
  }

  MiLightRadio* radio = milightClient->switchRadio(currentRadioType++ % milightClient->getNumRadios());

  for (size_t i = 0; i < settings.listenRepeats; i++) {
    if (milightClient->available()) {
      uint8_t readPacket[MILIGHT_MAX_PACKET_LENGTH];
      size_t packetLen = milightClient->read(readPacket);

      const MiLightRemoteConfig* remoteConfig = MiLightRemoteConfig::fromReceivedPacket(
        radio->config(),
        readPacket,
        packetLen
      );

      if (remoteConfig == NULL) {
        // This can happen under normal circumstances, so not an error condition
#ifdef DEBUG_PRINTF
        Serial.println(F("WARNING: Couldn't find remote for received packet"));
#endif
        return;
      }

      onPacketSentHandler(readPacket, *remoteConfig);
    }
  }
}

/**
 * Called when MqttClient#update is first being processed.  Stop sending updates
 * and aggregate state changes until the update is finished.
 */
void onUpdateBegin() {
  if (bulbStateUpdater) {
    bulbStateUpdater->disable();
  }
}

/**
 * Called when MqttClient#update is finished processing.  Re-enable state
 * updates, which will flush accumulated state changes.
 */
void onUpdateEnd() {
  if (bulbStateUpdater) {
    bulbStateUpdater->enable();
  }
}

/**
 * Apply what's in the Settings object.
 */
void applySettings() {
  if (milightClient) {
    delete milightClient;
  }
  if (radioFactory) {
    delete radioFactory;
  }
  if (mqttClient) {
    delete mqttClient;
    delete bulbStateUpdater;

    mqttClient = NULL;
    bulbStateUpdater = NULL;
  }
  if (stateStore) {
    delete stateStore;
  }

  radioFactory = MiLightRadioFactory::fromSettings(settings);

  if (radioFactory == NULL) {
    Serial.println(F("ERROR: unable to construct radio factory"));
  }

  stateStore = new GroupStateStore(MILIGHT_MAX_STATE_ITEMS, settings.stateFlushInterval);

  milightClient = new MiLightClient(
    radioFactory,
    *stateStore,
    settings.packetRepeatThrottleThreshold,
    settings.packetRepeatThrottleSensitivity,
    settings.packetRepeatMinimum
  );
  milightClient->begin();
  milightClient->onPacketSent(onPacketSentHandler);
  milightClient->onUpdateBegin(onUpdateBegin);
  milightClient->onUpdateEnd(onUpdateEnd);
  milightClient->setResendCount(settings.packetRepeats);

  if (settings.mqttServer().length() > 0) {
    mqttClient = new MqttClient(settings, milightClient);
    mqttClient->begin();
    bulbStateUpdater = new BulbStateUpdater(settings, *mqttClient, *stateStore);
  }

  initMilightUdpServers();

  if (discoveryServer) {
    delete discoveryServer;
    discoveryServer = NULL;
  }
  if (settings.discoveryPort != 0) {
    discoveryServer = new MiLightDiscoveryServer(settings);
    discoveryServer->begin();
  }
}

/**
 *
 */
bool shouldRestart() {
  if (! settings.isAutoRestartEnabled()) {
    return false;
  }

  return settings.getAutoRestartPeriod()*60*1000 < millis();
}

void setup() {
  Serial.begin(9600);
  String ssid = "ESP" + String(ESP.getChipId());

  wifiManager.setConfigPortalTimeout(180);
  wifiManager.autoConnect(ssid.c_str(), "milightHub");

  SPIFFS.begin();
  Settings::load(settings);
  applySettings();

  if (! MDNS.begin("milight-hub")) {
    Serial.println(F("Error setting up MDNS responder"));
  }

  MDNS.addService("http", "tcp", 80);

  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("ESP8266 MiLight Gateway");
  SSDP.setSerialNumber(ESP.getChipId());
  SSDP.setURL("/");
  SSDP.setDeviceType("upnp:rootdevice");
  SSDP.begin();

  httpServer = new MiLightHttpServer(settings, milightClient, stateStore);
  httpServer->onSettingsSaved(applySettings);
  httpServer->on("/description.xml", HTTP_GET, []() { SSDP.schema(httpServer->client()); });
  httpServer->begin();

  Serial.println(F("Setup complete"));
}

void loop() {
  httpServer->handleClient();

  if (mqttClient) {
    mqttClient->handleClient();
    bulbStateUpdater->loop();
  }

  if (udpServers) {
    for (size_t i = 0; i < settings.numGatewayConfigs; i++) {
      udpServers[i]->handleClient();
    }
  }

  if (discoveryServer) {
    discoveryServer->handleClient();
  }

  handleListen();

  stateStore->limitedFlush();

  if (shouldRestart()) {
    Serial.println(F("Auto-restart triggered. Restarting..."));
    ESP.restart();
  }
}
