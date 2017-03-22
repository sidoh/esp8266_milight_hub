// This protocol is documented here:
// http://www.limitlessled.com/dev/

#include <Arduino.h>
#include <MiLightClient.h>
#include <WiFiUdp.h>
#include <MiLightUdpServer.h>

#ifndef _V6_MILIGHT_UDP_SERVER
#define _V6_MILIGHT_UDP_SERVER 

class V6MiLightUdpServer : public MiLightUdpServer {
public:
  V6MiLightUdpServer(MiLightClient*& client, uint16_t port, uint16_t deviceId)
    : MiLightUdpServer(client, port, deviceId)
  { }
  
  // Should return size of the response packet
  virtual size_t handlePacket(uint8_t* packet, size_t packetSize, uint8_t* responseBuffer);
    
protected:
};

#endif
