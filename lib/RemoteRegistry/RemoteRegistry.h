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

struct GestureConfig {
  bool shortEnabled = true;
  bool longEnabled = true;
  bool doubleEnabled = false;
  bool tripleEnabled = false;
  uint16_t multiTapWindowMs = 360;
};

struct SurfaceSegmentConfig {
  bool enabled = false;
  uint8_t segments = 4;       // 2..12 for hue_ring, 2..6 for linear bars
  int16_t offsetDeg = 0;      // Rotation offset for hue_ring (e.g. -22 or -30 to center slice 1 at top)
  uint16_t zoneMask = 0xFFFF; // Bitmask of zones (bit 0 = Zone 0, bit 1 = Zone 1 ... 0xFFFF = All zones)
  String labels[12];
};

struct RegisteredRemote {
  uint16_t deviceId = 0;
  String remoteType = "fut089";
  String visualModel = "FUT089";
  String name = "Remote";
  String topicPrefix = "milight/actions/remote";
  uint8_t numZones = 8;
  bool active = true;
  GestureConfig gestures;
  SurfaceSegmentConfig hueRing;
  SurfaceSegmentConfig cctBar;
  SurfaceSegmentConfig satBar;
  SurfaceSegmentConfig briBar;
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
  uint32_t seq = 0;
  uint32_t timestampMs = 0;
  uint16_t deviceId = 0;
  uint8_t groupId = 0;
  String remoteType;
  String action;
  String gesture = "single";
  String label;
  uint8_t segment = 0;
  uint8_t segmentsTotal = 0;
  int32_t value = 0;
  String v2Dec;
  String source;
  String rawJson;
};

struct PendingTapTracker {
  bool active = false;
  uint16_t deviceId = 0;
  uint8_t groupId = 0;
  String remoteType;
  String baseAction;
  int32_t value = -1;
  bool fromVirtual = false;
  uint8_t count = 0;
  uint32_t lastTapMs = 0;
  uint32_t deadlineMs = 0;
};

struct PendingSegmentSettle {
  bool active = false;
  uint16_t deviceId = 0;
  uint8_t groupId = 0;
  String remoteType;
  String surface; // "hue_ring", "cct_bar", "sat_bar", "bri_bar"
  uint8_t segment = 1;
  uint8_t segmentsTotal = 4;
  String label;
  int32_t rawValue = 0;
  bool fromVirtual = false;
  uint32_t settleDeadlineMs = 0;
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
  static void parseRemoteFromJson(const JsonObject& obj, RegisteredRemote& r);
  static void writeRemoteToJson(const RegisteredRemote& r, JsonObject& obj, bool compactKeys = false);

  uint8_t resolveGroupId(uint16_t deviceId, uint8_t rawGroupId, uint8_t protocolMaxZones);
  bool hasTypeOverride(uint16_t deviceId, MiLightRemoteType& outType) const;
  void handlePacket(uint16_t deviceId, uint8_t groupId, const String& remoteTypeName, const JsonObject& result, bool fromVirtual = false);
  bool virtualSend(uint16_t deviceId, const String& remoteTypeName, uint8_t groupId, const String& action, int32_t value);

  void serializeRemotes(JsonArray arr) const;
  void serializeLastSeen(JsonObject obj) const;

  std::vector<RegisteredRemote> remotes;
  LastSeenRemoteEvent lastSeen;
  static constexpr size_t RING_CAP = 8;
  LastSeenRemoteEvent recentRing[RING_CAP];
  size_t ringHead = 0;
  size_t ringCount = 0;

private:
  RemoteRegistry();
  MqttClient** mqttClientPtr;
  MiLightClient** milightClientPtr;
  RawPublishFn fallbackPub;
  void sendMqtt(const char* topic, const char* payload, bool retain);
  void emitResolvedEvent(
    const RegisteredRemote* r,
    uint16_t deviceId,
    uint8_t groupId,
    const String& remoteTypeName,
    const String& action,
    const String& gesture,
    int32_t value,
    bool fromVirtual,
    const String& surface = "",
    uint8_t segment = 0,
    uint8_t segmentsTotal = 0,
    const String& segLabel = "",
    const String& v2Dec = ""
  );
  uint8_t computeHueSegment(int32_t hueDeg, const SurfaceSegmentConfig& cfg, uint32_t stateKey);
  uint8_t computeLinearSegment(int32_t normPct, const SurfaceSegmentConfig& cfg, uint32_t stateKey);

  bool loaded;
  bool discoverySent;
  uint32_t lastDiscRetryMs;
  uint32_t lastDedupSig;
  uint32_t lastDedupMs;
  std::map<uint16_t, uint8_t> lastActiveZoneByDev;
  std::map<uint32_t, AccumulatedZoneState> zoneStates;
  std::map<uint32_t, PendingTapTracker> tapTrackers;
  std::map<uint32_t, PendingSegmentSettle> segSettleTrackers;
  std::map<uint32_t, uint8_t> lastSegmentBySurface;
};

#endif
