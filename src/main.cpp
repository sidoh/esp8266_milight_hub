#include <SPI.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <FS.h>
#include <GithubClient.h>
#include <IntParsing.h>
#include <Size.h>
#include <MiLightClient.h>
#include <MiLightRadioConfig.h>
#include <MiLightHttpServer.h>
#include <Settings.h>
#include <MiLightUdpServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266SSDP.h>
#include <MqttClient.h>
#include <RGBConverter.h>

WiFiManager wifiManager;

Settings settings;

MiLightClient* milightClient;
MiLightRadioFactory* radioFactory;
MiLightHttpServer *httpServer;
MqttClient* mqttClient;

int numUdpServers = 0;
MiLightUdpServer** udpServers;

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

  if (settings.mqttServer().length() > 0) {
    mqttClient = new MqttClient(settings, milightClient);
    mqttClient->begin();
  }

  initMilightUdpServers();
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

  if (shouldRestart()) {
    Serial.println(F("Auto-restart triggered. Restarting..."));
    ESP.restart();
  }
}
