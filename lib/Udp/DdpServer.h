#ifndef _DDP_SERVER_H
#define _DDP_SERVER_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <MiLightClient.h>
#include <MiLightRadioConfig.h>

#define DDP_PORT 4048
#define DDP_HEADER_LEN 10
#define DDP_NUM_ZONES 8

struct ZoneColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t hue;
  uint8_t brightness;
  bool isDifferent(const ZoneColor& other, uint8_t threshold) const {
    return (abs((int)hue - (int)other.hue) > threshold) ||
           (abs((int)brightness - (int)other.brightness) > threshold);
  }
};

class DdpServer {
public:
  DdpServer(MiLightClient*& client, uint16_t deviceId);
  void begin();
  void handlePacket();

private:
  WiFiUDP udp;
  MiLightClient*& client;
  uint16_t deviceId;

  uint8_t packetBuffer[128];
  ZoneColor lastZoneState[DDP_NUM_ZONES];
  uint32_t lastTransmitMs;

  void parseRgbToMiLight(uint8_t r, uint8_t g, uint8_t b, uint8_t& outHue, uint8_t& outBrightness);
  void processDdpFrame(const uint8_t* payload, size_t length);
};

#endif
