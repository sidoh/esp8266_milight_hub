// This protocol is documented here:
// http://www.limitlessled.com/dev/

#include <Arduino.h>
#include <MiLightClient.h>
#include <WiFiUdp.h>
#include <MiLightUdpServer.h>

#ifndef _V5_MILIGHT_UDP_SERVER
#define _V5_MILIGHT_UDP_SERVER

enum MiLightUdpCommands {
  UDP_CCT_ALL_ON             = 0x35,
  UDP_CCT_ALL_OFF            = 0x39,
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
  UDP_CCT_NIGHT_MODE         = 0xB9,

  UDP_RGBW_ALL_OFF           = 0x41,
  UDP_RGBW_ALL_ON            = 0x42,
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
  UDP_RGBW_GROUP_ALL_NIGHT   = 0xC1,
  UDP_RGBW_GROUP_1_NIGHT     = 0xC6,
  UDP_RGBW_GROUP_2_NIGHT     = 0xC8,
  UDP_RGBW_GROUP_3_NIGHT     = 0xCA,
  UDP_RGBW_GROUP_4_NIGHT     = 0xCC,
  UDP_RGBW_BRIGHTNESS        = 0x4E,
  UDP_RGBW_COLOR             = 0x40
};

class V5MiLightUdpServer : public MiLightUdpServer {
public:
  V5MiLightUdpServer(MiLightClient*& client, uint16_t port, uint16_t deviceId)
    : MiLightUdpServer(client, port, deviceId)
  { }

  // Should return size of the response packet
  virtual void handlePacket(uint8_t* packet, size_t packetSize);

protected:
  void handleCommand(uint8_t command, uint8_t commandArg);
  void pressButton(uint8_t button);
};

#endif
