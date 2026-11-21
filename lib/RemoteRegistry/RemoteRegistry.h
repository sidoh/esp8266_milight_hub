#ifndef _REMOTE_REGISTRY_H
#define _REMOTE_REGISTRY_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <functional>
#include <map>
#include <MqttClient.h>
#include <MiLightClient.h>
#include <MiLightRemoteType.h>

struct RegisteredRemote {
  uint16_t deviceId;
  String remoteType;
  String visualModel;
  String name;
  String topicPrefix;
  uint8_t numZones;
  bool active;
};

struct AccumulatedZoneState {
  bool on = true;
  int16_t brightness = -1;
  int16_t hue = -1;
  int16_t saturation = -1;
  int16_t colorTemp = -1;
  int16_t mode = -1;
  String colorMode = "white";
  String lastAction = "none";
};

struct LastSeenRemoteEvent {
  uint32_t seq;
  uint32_t timestampMs;
  uint16_t deviceId;
  uint8_t groupId;
  String remoteType;
  String action;
  int32_t value;
  String v2Dec;
  String source;
  String rawJson;
};

class RemoteRegistry {
public:
  using RawPublishFn = std::function<bool(const char*, const char*, bool)>;
  static RemoteRegistry& instance();
  void setFallbackPublisher(RawPublishFn fn);
  void tick();

  void begin(MqttClient** mqttClientPtr, MiLightClient** milightClientPtr);
  void load();
  void save();
  void publishDiscovery();

  const RegisteredRemote* find(uint16_t deviceId) const;
  bool upsert(const RegisteredRemote& item);
  bool remove(uint16_t deviceId);

  uint8_t resolveGroupId(uint16_t deviceId, uint8_t rawGroupId, uint8_t protocolMaxZones);
  bool hasTypeOverride(uint16_t deviceId, MiLightRemoteType& outType) const;
  void handlePacket(uint16_t deviceId, uint8_t groupId, const String& remoteTypeName, const JsonObject& result, bool fromVirtual = false);
  bool virtualSend(uint16_t deviceId, const String& remoteTypeName, uint8_t groupId, const String& action, int32_t value);

  void serializeRemotes(JsonArray arr) const;
  void serializeLastSeen(JsonObject obj) const;

  std::vector<RegisteredRemote> remotes;
  LastSeenRemoteEvent lastSeen;
  static constexpr size_t RING_CAP = 6;
  LastSeenRemoteEvent recentRing[RING_CAP];
  size_t ringHead = 0;
  size_t ringCount = 0;

private:
  RemoteRegistry();
  MqttClient** mqttClientPtr;
  MiLightClient** milightClientPtr;
  RawPublishFn fallbackPub;
  void sendMqtt(const char* topic, const char* payload, bool retain);
  bool loaded;
  bool discoverySent;
  uint32_t lastDiscRetryMs;
  uint32_t lastDedupSig;
  uint32_t lastDedupMs;
  std::map<uint16_t, uint8_t> lastActiveZoneByDev;
  std::map<uint32_t, AccumulatedZoneState> zoneStates;
};

#endif
