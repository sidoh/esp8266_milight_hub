#pragma once

#include <stdint.h>
#include <MiLightRemoteType.h>
#include <ArduinoJson.h>

struct BulbId {
  uint16_t deviceId;
  uint8_t groupId;
  MiLightRemoteType deviceType;

  BulbId();
  BulbId(const BulbId& other);
  BulbId(const uint16_t deviceId, const uint8_t groupId, const MiLightRemoteType deviceType);
  bool operator==(const BulbId& other);
  void operator=(const BulbId& other);

  uint32_t getCompactId() const;
  String getHexDeviceId() const;
  void serialize(JsonObject json) const;
  void serialize(JsonArray json) const;
};