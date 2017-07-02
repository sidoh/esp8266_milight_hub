#include <SPI.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <FS.h>
#include <GithubClient.h>
#include <IntParsing.h>
#include <Size.h>
#include <MiLightRadioConfig.h>
#include <MiLightHttpServer.h>
#include <Settings.h>
#include <MiLightUdpServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266SSDP.h>
#include <MqttClient.h>
#include <RGBConverter.h>
#include <MiLightDiscoveryServer.h>
#include <MiLightClient.h>

WiFiManager wifiManager;

Settings settings;

MiLightClient* milightClient = NULL;
MiLightRadioFactory* radioFactory = NULL;
MiLightHttpServer *httpServer = NULL;
MqttClient* mqttClient = NULL;
MiLightDiscoveryServer* discoveryServer = NULL;
uint8_t currentRadioType = 0;

int numUdpServers = 0;
MiLightUdpServer** udpServers;
WiFiUDP udpSeder;

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

void onPacketSentHandler(uint8_t* packet, const MiLightRadioConfig& config) {
  StaticJsonBuffer<200> buffer;
  JsonObject& result = buffer.createObject();
  config.packetFormatter->parsePacket(packet, result);

  uint16_t deviceId = result["device_id"];
  uint16_t groupId = result["group_id"];
  MiLightRadioType type = MiLightRadioConfig::fromString(result["device_type"])->type;

  char output[200];
  result.printTo(output);

  if (mqttClient) {
    mqttClient->sendUpdate(type, deviceId, groupId, output);
  }
}

void handleListen() {
  if (! settings.listenRepeats) {
    return;
  }

  MiLightRadioConfig* config = MiLightRadioConfig::ALL_CONFIGS[
    currentRadioType++ % MiLightRadioConfig::NUM_CONFIGS
  ];
  milightClient->prepare(*config);

  for (size_t i = 0; i < settings.listenRepeats; i++) {
    if (milightClient->available()) {
      uint8_t readPacket[9];
      milightClient->read(readPacket);

      onPacketSentHandler(readPacket, *config);
    }
  }
}

void applySettings() {
  if (milightClient) {
    delete milightClient;
  }
  if (radioFactory) {
    delete radioFactory;
  }
  if (mqttClient) {
    delete mqttClient;
  }

  radioFactory = MiLightRadioFactory::fromSettings(settings);

  if (radioFactory == NULL) {
    Serial.println(F("ERROR: unable to construct radio factory"));
  }

  milightClient = new MiLightClient(radioFactory);
  milightClient->begin();
  milightClient->onPacketSent(onPacketSentHandler);

  if (settings.mqttServer().length() > 0) {
    mqttClient = new MqttClient(settings, milightClient);
    mqttClient->begin();
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

bool shouldRestart() {
  if (! settings.isAutoRestartEnabled()) {
    return false;
  }

  return settings.getAutoRestartPeriod()*60*1000 < millis();
}

void setup() {
  Serial.begin(9600);
  wifiManager.autoConnect();
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

  httpServer = new MiLightHttpServer(settings, milightClient);
  httpServer->onSettingsSaved(applySettings);
  httpServer->on("/description.xml", HTTP_GET, []() { SSDP.schema(httpServer->client()); });
  httpServer->begin();

  Serial.println(F("Setup complete"));
}

void loop() {
  httpServer->handleClient();

  if (mqttClient) {
    mqttClient->handleClient();
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

  if (shouldRestart()) {
    Serial.println(F("Auto-restart triggered. Restarting..."));
    ESP.restart();
  }
}
