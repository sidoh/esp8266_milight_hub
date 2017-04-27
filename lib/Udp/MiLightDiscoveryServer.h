#include <WiFiUdp.h>
#include <Settings.h>

#ifndef MILIGHT_DISCOVERY_SERVER_H
#define MILIGHT_DISCOVERY_SERVER_H

class MiLightDiscoveryServer {
public:
  MiLightDiscoveryServer(Settings& settings);
  MiLightDiscoveryServer(MiLightDiscoveryServer&);
  MiLightDiscoveryServer& operator=(MiLightDiscoveryServer other);
  ~MiLightDiscoveryServer();

  void begin();
  void handleClient();

private:
  Settings& settings;
  WiFiUDP socket;

  void handleDiscovery(uint8_t version);
  void sendResponse(char* buffer);
};

#endif
