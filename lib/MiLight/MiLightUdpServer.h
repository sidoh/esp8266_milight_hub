#include <Arduino.h>
#include <MiLightClient.h>
#include <WiFiUdp.h>

#define MILIGHT_PACKET_BUFFER_SIZE 10

#ifndef _MILIGHT_UDP_SERVER
#define _MILIGHT_UDP_SERVER 

// These are mostly a remapping of MiLightButton
enum MiLightUdpCommands {
  UDP_ALL_ON            = 0x41,
  UDP_ALL_OFF           = 0x42,
  
  UDP_SPEED_UP          = 0x43, 
  UDP_SPEED_DOWN        = 0x44, 
  
  UDP_GROUP_1_ON        = 0x45,
  UDP_GROUP_1_OFF       = 0x46,
  UDP_GROUP_2_ON        = 0x47,
  UDP_GROUP_2_OFF       = 0x48,
  UDP_GROUP_3_ON        = 0x49,
  UDP_GROUP_3_OFF       = 0x4A,
  UDP_GROUP_4_ON        = 0x4B,
  UDP_GROUP_4_OFF       = 0x4C,
  
  UDP_DISCO_MODE        = 0x4D,
  
  UDP_GROUP_ALL_WHITE   = 0xC2,
  UDP_GROUP_1_WHITE     = 0xC5,
  UDP_GROUP_2_WHITE     = 0xC7,
  UDP_GROUP_3_WHITE     = 0xC9,
  UDP_GROUP_4_WHITE     = 0xCB,
  
  UDP_BRIGHTNESS        = 0x4E,
  UDP_COLOR             = 0x40
};

class MiLightUdpServer {
public:
  MiLightUdpServer(MiLightClient*& client, uint16_t port, uint16_t deviceId);
    
  void begin();
  void handleClient();
    
protected:
  WiFiUDP socket;
  MiLightClient*& client;
  uint16_t port;
  uint16_t deviceId;
  uint8_t lastGroup;
  char packetBuffer[MILIGHT_PACKET_BUFFER_SIZE];
  
  void handleCommand(uint8_t command, uint8_t commandArg);
  void pressButton(uint8_t group, MiLightButton button);
};

#endif