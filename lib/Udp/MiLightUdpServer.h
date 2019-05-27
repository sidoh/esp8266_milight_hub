#include <Arduino.h>
#include <MiLightClient.h>
#include <WiFiUdp.h>

#include <memory>

// This protocol is documented here:
// http://www.limitlessled.com/dev/

#define MILIGHT_PACKET_BUFFER_SIZE 30

// Uncomment to enable Serial printing of packets
// #define MILIGHT_UDP_DEBUG

#ifndef _MILIGHT_UDP_SERVER
#define _MILIGHT_UDP_SERVER

class MiLightUdpServer {
public:
  MiLightUdpServer(MiLightClient*& client, uint16_t port, uint16_t deviceId);
  virtual ~MiLightUdpServer();

  void stop();
  void begin();
  void handleClient();

  static std::shared_ptr<MiLightUdpServer> fromVersion(uint8_t version, MiLightClient*&, uint16_t port, uint16_t deviceId);

protected:
  WiFiUDP socket;
  MiLightClient*& client;
  uint16_t port;
  uint16_t deviceId;
  uint8_t lastGroup;
  uint8_t packetBuffer[MILIGHT_PACKET_BUFFER_SIZE];
  uint8_t responseBuffer[MILIGHT_PACKET_BUFFER_SIZE];

  // Should return size of the response packet
  virtual void handlePacket(uint8_t* packet, size_t packetSize) = 0;
};

#endif