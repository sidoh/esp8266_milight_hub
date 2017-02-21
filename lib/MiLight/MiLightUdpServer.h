#include <Arduino.h>
#include <MiLightClient.h>
#include <WiFiUdp.h>

// This protocol is documented here:
// http://www.limitlessled.com/dev/

#define MILIGHT_PACKET_BUFFER_SIZE 10

#ifndef _MILIGHT_UDP_SERVER
#define _MILIGHT_UDP_SERVER 

enum MiLightUdpCommands {
  UDP_CCT_GROUP_1_ON         = 0x38,
  UDP_CCT_GROUP_1_OFF        = 0x3B,
  UDP_CCT_GROUP_2_ON         = 0x3D,
  UDP_CCT_GROUP_2_OFF        = 0x33,
  UDP_CCT_GROUP_3_ON         = 0x37,
  UDP_CCT_GROUP_3_OFF        = 0x3A,
  UDP_CCT_GROUP_4_ON         = 0x32,
  UDP_CCT_GROUP_4_OFF        = 0x36,
  UDP_CCT_TEMPERATURE_DOWN   = 0x3F,
  UDP_CCT_TEMPERATURE_UP     = 0x3E,
  UDP_CCT_BRIGHTNESS_DOWN    = 0x34,
  UDP_CCT_BRIGHTNESS_UP      = 0x3C,
  
  UDP_RGBW_ALL_ON            = 0x41,
  UDP_RGBW_ALL_OFF           = 0x42,
  UDP_RGBW_SPEED_UP          = 0x43, 
  UDP_RGBW_SPEED_DOWN        = 0x44, 
  UDP_RGBW_GROUP_1_ON        = 0x45,
  UDP_RGBW_GROUP_1_OFF       = 0x46,
  UDP_RGBW_GROUP_2_ON        = 0x47,
  UDP_RGBW_GROUP_2_OFF       = 0x48,
  UDP_RGBW_GROUP_3_ON        = 0x49,
  UDP_RGBW_GROUP_3_OFF       = 0x4A,
  UDP_RGBW_GROUP_4_ON        = 0x4B,
  UDP_RGBW_GROUP_4_OFF       = 0x4C,
  UDP_RGBW_DISCO_MODE        = 0x4D,
  UDP_RGBW_GROUP_ALL_WHITE   = 0xC2,
  UDP_RGBW_GROUP_1_WHITE     = 0xC5,
  UDP_RGBW_GROUP_2_WHITE     = 0xC7,
  UDP_RGBW_GROUP_3_WHITE     = 0xC9,
  UDP_RGBW_GROUP_4_WHITE     = 0xCB,
  UDP_RGBW_BRIGHTNESS        = 0x4E,
  UDP_RGBW_COLOR             = 0x40
};

class MiLightUdpServer {
public:
  MiLightUdpServer(MiLightClient*& client, uint16_t port, uint16_t deviceId);
  ~MiLightUdpServer();
    
  void stop();
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
  void pressButton(uint8_t group, uint8_t button);
  uint8_t cctCommandIdToGroup(uint8_t command);
  MiLightStatus cctCommandToStatus(uint8_t command);
};

#endif