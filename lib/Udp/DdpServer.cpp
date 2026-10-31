#include <DdpServer.h>

DdpServer::DdpServer(MiLightClient*& client, uint16_t deviceId)
  : client(client),
    deviceId(deviceId),
    lastTransmitMs(0)
{
  memset(lastZoneState, 0, sizeof(lastZoneState));
}

void DdpServer::begin() {
  udp.begin(DDP_PORT);
}

void DdpServer::parseRgbToMiLight(uint8_t r, uint8_t g, uint8_t b, uint8_t& outHue, uint8_t& outBrightness) {
  uint8_t maxVal = max(r, max(g, b));
  uint8_t minVal = min(r, min(g, b));
  uint8_t delta = maxVal - minVal;

  outBrightness = maxVal;

  if (delta == 0) {
    outHue = 0;
    return;
  }

  int16_t h = 0;
  if (maxVal == r) {
    h = ((int16_t)(g - b) * 60) / delta;
    if (h < 0) h += 360;
  } else if (maxVal == g) {
    h = 120 + ((int16_t)(b - r) * 60) / delta;
  } else {
    h = 240 + ((int16_t)(r - g) * 60) / delta;
  }

  outHue = (uint8_t)(((360 - h + 240) % 360) * 255 / 360);
}

void DdpServer::processDdpFrame(const uint8_t* payload, size_t length) {
  if (length < (DDP_NUM_ZONES * 3)) {
    return;
  }

  ZoneColor current[DDP_NUM_ZONES];
  bool allZonesIdentical = true;

  for (size_t z = 0; z < DDP_NUM_ZONES; z++) {
    uint8_t r = payload[z * 3];
    uint8_t g = payload[z * 3 + 1];
    uint8_t b = payload[z * 3 + 2];
    current[z].r = r;
    current[z].g = g;
    current[z].b = b;
    parseRgbToMiLight(r, g, b, current[z].hue, current[z].brightness);

    if (z > 0) {
      if (current[z].hue != current[0].hue || current[z].brightness != current[0].brightness) {
        allZonesIdentical = false;
      }
    }
  }

  if (allZonesIdentical && current[0].isDifferent(lastZoneState[0], 10)) {
    client->prepare(&FUT089Config, deviceId, 0);
    if (current[0].brightness == 0) {
      client->updateStatus(OFF);
    } else {
      client->updateColorRaw(current[0].hue);
      client->updateBrightness(current[0].brightness);
    }
    for (size_t z = 0; z < DDP_NUM_ZONES; z++) {
      lastZoneState[z] = current[z];
    }
    return;
  }

  for (size_t z = 0; z < DDP_NUM_ZONES; z++) {
    if (current[z].isDifferent(lastZoneState[z], 15)) {
      client->prepare(&FUT089Config, deviceId, z + 1);

      if (current[z].brightness == 0) {
        client->updateStatus(OFF);
      } else {
        if (abs((int)current[z].hue - (int)lastZoneState[z].hue) > 15) {
          client->updateColorRaw(current[z].hue);
        }
        if (abs((int)current[z].brightness - (int)lastZoneState[z].brightness) > 20) {
          client->updateBrightness(current[z].brightness);
        }
      }
      lastZoneState[z] = current[z];
    }
  }
}

void DdpServer::handlePacket() {
  int packetSize = udp.parsePacket();
  if (!packetSize) {
    return;
  }

  int len = udp.read(packetBuffer, sizeof(packetBuffer));
  if (len > DDP_HEADER_LEN) {
    if ((packetBuffer[0] & 0x40) == 0x40) {
      processDdpFrame(packetBuffer + DDP_HEADER_LEN, len - DDP_HEADER_LEN);
    }
  }
}
