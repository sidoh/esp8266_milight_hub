#include <SPI.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <fs.h>
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
    
    if (config->protocolVersion == 0) {
      udpServers[i] = new MiLightUdpServer(milightClient, config->port, config->deviceId);
      udpServers[i]->begin();
    } else {
      Serial.print("Error initializing milight UDP server - Unsupported protocolVersion: ");
      Serial.println(config->protocolVersion);
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
  Serial.println(" Init client");
  initMilightClient();
  Serial.println(" Init UDP servers");
  initMilightUdpServers();
}

void setup() {
  Serial.begin(9600);
  wifiManager.autoConnect();
  SPIFFS.begin();
  Settings::load(settings);
  Serial.println("Applying settings");
  applySettings();
  Serial.println("Done");
  
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
