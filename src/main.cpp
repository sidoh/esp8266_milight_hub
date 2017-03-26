#include <SPI.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <FS.h>
#include <IntParsing.h>
#include <MiLightClient.h>
#include <MiLightRadioConfig.h>
#include <MiLightHttpServer.h>
#include <Settings.h>
#include <MiLightUdpServer.h>

WiFiManager wifiManager;

Settings settings;

MiLightClient* milightClient;
MiLightHttpServer *httpServer;

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
      Serial.print("Error creating UDP server with protocol version: ");
      Serial.println(config->protocolVersion);
    } else {
      udpServers[i] = server;
      udpServers[i]->begin();
    }
  }
}

void initMilightClient() {
  if (milightClient) {
    delete milightClient;
  }
  
  milightClient = new MiLightClient(settings.cePin, settings.csnPin);
  milightClient->begin();
}

void applySettings() {
  initMilightClient();
  initMilightUdpServers();
}

void setup() {
  Serial.begin(9600);
  wifiManager.autoConnect();
  SPIFFS.begin();
  Settings::load(settings);
  applySettings();
  
  httpServer = new MiLightHttpServer(settings, milightClient);
  httpServer->onSettingsSaved(applySettings);
  httpServer->begin();
}

void loop() {
  httpServer->handleClient();
  
  if (udpServers) {
    for (size_t i = 0; i < settings.numGatewayConfigs; i++) {
      udpServers[i]->handleClient();
    }
  }
}
