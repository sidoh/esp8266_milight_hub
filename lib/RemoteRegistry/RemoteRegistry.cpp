#include <RemoteRegistry.h>
#include <FS.h>
#ifdef ESP32
  #include <SPIFFS.h>
#endif
#include <ProjectFS.h>
extern bool initialized;

static void parseSurfaceJson(const JsonObject& sObj, SurfaceSegmentConfig& sc, uint8_t maxSegs, uint8_t defSegs) {
  if (sObj.isNull()) return;
  sc.enabled = sObj["enabled"] | false;
  int segs = sObj["segments"] | defSegs;
  sc.segments = (uint8_t)constrain(segs, 2, (int)maxSegs);
  sc.offsetDeg = (int16_t)constrain((int)(sObj["offset_deg"] | 0), -180, 180);
  sc.zoneMask = (uint16_t)(sObj["zone_mask"] | 0xFFFF);
  if (sObj.containsKey("labels") && sObj["labels"].is<JsonArray>()) {
    JsonArray lArr = sObj["labels"].as<JsonArray>();
    for (size_t i = 0; i < 12; i++) {
      sc.labels[i] = (i < lArr.size()) ? String(lArr[i].as<const char*>() ? lArr[i].as<const char*>() : "") : "";
    }
  }
}

static void writeSurfaceJson(const SurfaceSegmentConfig& sc, JsonObject& sObj) {
  sObj["enabled"] = sc.enabled;
  sObj["segments"] = sc.segments;
  if (sc.offsetDeg != 0) sObj["offset_deg"] = sc.offsetDeg;
  sObj["zone_mask"] = sc.zoneMask;
  JsonArray lArr = sObj.createNestedArray("labels");
  for (uint8_t i = 0; i < sc.segments && i < 12; i++) {
    lArr.add(sc.labels[i]);
  }
}

void RemoteRegistry::parseRemoteFromJson(const JsonObject& obj, RegisteredRemote& r) {
  r.deviceId = obj["device_id"] | (obj["id"] | 0);
  r.remoteType = obj["remote_type"] | (obj["type"] | "fut089");
  r.visualModel = obj["visual_model"] | (obj["model"] | "FUT089");
  r.name = obj["name"] | "Remote";
  r.topicPrefix = obj["topic_prefix"] | (obj["prefix"] | "milight/actions/remote");
  r.numZones = obj["num_zones"] | (obj["zones"] | 8);
  r.active = obj["active"] | true;

  if (obj.containsKey("gestures") && obj["gestures"].is<JsonObject>()) {
    JsonObject g = obj["gestures"].as<JsonObject>();
    r.gestures.shortEnabled = g["short"] | true;
    r.gestures.longEnabled = g["long"] | true;
    r.gestures.doubleEnabled = g["double"] | false;
    r.gestures.tripleEnabled = g["triple"] | false;
    r.gestures.multiTapWindowMs = (uint16_t)constrain((int)(g["window_ms"] | 360), 180, 800);
  }

  if (obj.containsKey("child_lock") && obj["child_lock"].is<JsonObject>()) {
    JsonObject cl = obj["child_lock"].as<JsonObject>();
    r.childLock.enabled = cl["enabled"] | false;
    r.childLock.idleTimeoutSec = (uint16_t)constrain((int)(cl["timeout_sec"] | 300), 30, 7200);
    const char* seq = cl["sequence"] | "on_off_on";
    r.childLock.sequence = String(seq);
  }

  if (obj.containsKey("touch_zones") && obj["touch_zones"].is<JsonObject>()) {
    JsonObject tz = obj["touch_zones"].as<JsonObject>();
    if (tz.containsKey("hue_ring")) parseSurfaceJson(tz["hue_ring"].as<JsonObject>(), r.hueRing, 12, 6);
    if (tz.containsKey("cct_bar"))  parseSurfaceJson(tz["cct_bar"].as<JsonObject>(),  r.cctBar,  6,  4);
    if (tz.containsKey("sat_bar"))  parseSurfaceJson(tz["sat_bar"].as<JsonObject>(),  r.satBar,  6,  4);
    if (tz.containsKey("bri_bar"))  parseSurfaceJson(tz["bri_bar"].as<JsonObject>(),  r.briBar,  6,  4);
  }
}

void RemoteRegistry::writeRemoteToJson(const RegisteredRemote& r, JsonObject& obj, bool compactKeys) {
  char hexBuf[10];
  snprintf(hexBuf, sizeof(hexBuf), "0x%04X", r.deviceId);
  if (compactKeys) {
    obj["id"] = r.deviceId;
    obj["hex"] = hexBuf;
    obj["type"] = r.remoteType;
    obj["model"] = r.visualModel;
    obj["name"] = r.name;
    obj["prefix"] = r.topicPrefix;
    obj["zones"] = r.numZones;
    obj["active"] = r.active;
  } else {
    obj["device_id"] = r.deviceId;
    obj["hex_id"] = hexBuf;
    obj["remote_type"] = r.remoteType;
    obj["visual_model"] = r.visualModel;
    obj["name"] = r.name;
    obj["topic_prefix"] = r.topicPrefix;
    obj["num_zones"] = r.numZones;
    obj["active"] = r.active;
  }

  JsonObject g = obj.createNestedObject("gestures");
  g["short"] = r.gestures.shortEnabled;
  g["long"] = r.gestures.longEnabled;
  g["double"] = r.gestures.doubleEnabled;
  g["triple"] = r.gestures.tripleEnabled;
  g["window_ms"] = r.gestures.multiTapWindowMs;

  JsonObject cl = obj.createNestedObject("child_lock");
  cl["enabled"] = r.childLock.enabled;
  cl["timeout_sec"] = r.childLock.idleTimeoutSec;
  cl["sequence"] = r.childLock.sequence;
  if (!compactKeys) {
    cl["locked"] = RemoteRegistry::instance().isRemoteLocked(r.deviceId);
  }

  JsonObject tz = obj.createNestedObject("touch_zones");
  JsonObject hr = tz.createNestedObject("hue_ring");
  writeSurfaceJson(r.hueRing, hr);
  JsonObject cb = tz.createNestedObject("cct_bar");
  writeSurfaceJson(r.cctBar, cb);
  JsonObject sb = tz.createNestedObject("sat_bar");
  writeSurfaceJson(r.satBar, sb);
  JsonObject bb = tz.createNestedObject("bri_bar");
  writeSurfaceJson(r.briBar, bb);
}

RemoteRegistry& RemoteRegistry::instance() {
  static RemoteRegistry inst;
  return inst;
}

RemoteRegistry::RemoteRegistry()
  : mqttClientPtr(nullptr)
  , milightClientPtr(nullptr)
  , fallbackPub(nullptr)
  , loaded(false)
  , discoverySent(false)
  , lastDiscRetryMs(0)
  , lastDedupSig(0xFFFFFFFF)
  , lastDedupMs(0)
{
  lastSeen.rawJson = "{}";
}

void RemoteRegistry::setFallbackPublisher(RawPublishFn fn) {
  this->fallbackPub = fn;
}

void RemoteRegistry::sendMqtt(const char* topic, const char* payload, bool retain) {
  if (!initialized) return;
  if (mqttClientPtr != nullptr && *mqttClientPtr != nullptr && (*mqttClientPtr)->isConnected()) {
    (*mqttClientPtr)->send(topic, payload, retain);
    return;
  }
  if (fallbackPub) {
    fallbackPub(topic, payload, retain);
  }
}

void RemoteRegistry::tick() {
  uint32_t now = millis();

  // 1. Process expired Multi-Tap gesture windows (1x / 2x / 3x)
  for (auto it = tapTrackers.begin(); it != tapTrackers.end(); ) {
    PendingTapTracker& tr = it->second;
    if (tr.active && (int32_t)(now - tr.deadlineMs) >= 0) {
      PendingTapTracker copy = tr;
      it = tapTrackers.erase(it);
      const RegisteredRemote* r = find(copy.deviceId);
      String gest = (copy.count >= 3) ? "triple" : ((copy.count == 2) ? "double" : "single");
      if (r != nullptr) {
        if (gest == "single" && !r->gestures.shortEnabled) continue;
        if (gest == "double" && !r->gestures.doubleEnabled) gest = r->gestures.shortEnabled ? "single" : "";
        if (gest == "triple" && !r->gestures.tripleEnabled) {
          gest = r->gestures.doubleEnabled ? "double" : (r->gestures.shortEnabled ? "single" : "");
        }
        if (gest.length() == 0) continue;
      }
      emitResolvedEvent(r, copy.deviceId, copy.groupId, copy.remoteType, copy.baseAction, gest, copy.value, copy.fromVirtual);
    } else {
      ++it;
    }
  }

  // 2. Process settled Segmented Ring / Slider touches (140ms touch settle)
  for (auto it = segSettleTrackers.begin(); it != segSettleTrackers.end(); ) {
    PendingSegmentSettle& ps = it->second;
    if (ps.active && (int32_t)(now - ps.settleDeadlineMs) >= 0) {
      PendingSegmentSettle copy = ps;
      it = segSettleTrackers.erase(it);
      const RegisteredRemote* r = find(copy.deviceId);
      String prefix = (copy.surface == "hue_ring") ? "hue_seg_" :
                      (copy.surface == "cct_bar")  ? "cct_seg_" :
                      (copy.surface == "sat_bar")  ? "sat_seg_" : "bri_seg_";
      String segAct = prefix + String(copy.segment);
      emitResolvedEvent(
        r, copy.deviceId, copy.groupId, copy.remoteType,
        segAct, "single", copy.rawValue, copy.fromVirtual,
        copy.surface, copy.segment, copy.segmentsTotal, copy.label
      );
    } else {
      ++it;
    }
  }

  // 3. Discovery retry until broker is online
  if (!discoverySent && (now - lastDiscRetryMs >= 3000)) {
    lastDiscRetryMs = now;
    bool hubReady = (mqttClientPtr != nullptr && *mqttClientPtr != nullptr && (*mqttClientPtr)->isConnected());
    if (hubReady) {
      publishDiscovery();
      discoverySent = true;
    } else if (fallbackPub && fallbackPub("milight/remotes_registry/status", "online", true)) {
      publishDiscovery();
      discoverySent = true;
    }
  }
}

void RemoteRegistry::begin(MqttClient** mqttPtr, MiLightClient** milightPtr) {
  this->mqttClientPtr = mqttPtr;
  this->milightClientPtr = milightPtr;
  if (!loaded) {
    load();
  }
  discoverySent = false;
  publishDiscovery();
}

void RemoteRegistry::load() {
  remotes.clear();
  loaded = true;

  if (!ProjectFS.exists("/remotes.json")) {
    RegisteredRemote defRem;
    defRem.deviceId = 0x125C;
    defRem.remoteType = "fut089";
    defRem.visualModel = "FUT089";
    defRem.name = "Main 8-Zone Remote";
    defRem.topicPrefix = "milight/actions/main_remote";
    defRem.numZones = 8;
    defRem.active = true;
    remotes.push_back(defRem);
    save();
    return;
  }

  File f = ProjectFS.open("/remotes.json", "r");
  if (!f) return;

  DynamicJsonDocument doc(8192);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err || !doc.is<JsonArray>()) return;

  for (JsonObject obj : doc.as<JsonArray>()) {
    RegisteredRemote r;
    parseRemoteFromJson(obj, r);
    if (r.deviceId != 0) {
      remotes.push_back(r);
    }
  }
}

void RemoteRegistry::save() {
  DynamicJsonDocument doc(8192);
  JsonArray arr = doc.to<JsonArray>();
  serializeRemotes(arr);

  File f = ProjectFS.open("/remotes.json", "w");
  if (f) {
    serializeJson(doc, f);
    f.close();
  }
  publishDiscovery();
}

void RemoteRegistry::serializeRemotes(JsonArray arr) const {
  for (const auto& r : remotes) {
    JsonObject obj = arr.createNestedObject();
    writeRemoteToJson(r, obj, false);
  }
}

void RemoteRegistry::serializeLastSeen(JsonObject obj) const {
  char hexBuf[10];
  snprintf(hexBuf, sizeof(hexBuf), "0x%04X", lastSeen.deviceId);
  obj["seq"] = lastSeen.seq;
  obj["age_ms"] = (lastSeen.seq > 0) ? (millis() - lastSeen.timestampMs) : 0;
  obj["device_id"] = lastSeen.deviceId;
  obj["hex_id"] = hexBuf;
  obj["group_id"] = lastSeen.groupId;
  obj["remote_type"] = lastSeen.remoteType;
  obj["action"] = lastSeen.action;
  obj["gesture"] = lastSeen.gesture;
  if (lastSeen.segment > 0) {
    obj["segment"] = lastSeen.segment;
    obj["segments_total"] = lastSeen.segmentsTotal;
    obj["label"] = lastSeen.label;
  }
  obj["value"] = lastSeen.value;
  obj["v2_dec"] = lastSeen.v2Dec;
  obj["source"] = lastSeen.source;
  obj["raw"] = lastSeen.rawJson;

  JsonArray arr = obj.createNestedArray("recent");
  size_t start = (ringCount < RING_CAP) ? 0 : ringHead;
  for (size_t i = 0; i < ringCount; i++) {
    const auto& ev = recentRing[(start + i) % RING_CAP];
    JsonObject eo = arr.createNestedObject();
    char hb[10];
    snprintf(hb, sizeof(hb), "0x%04X", ev.deviceId);
    eo["seq"] = ev.seq;
    eo["device_id"] = ev.deviceId;
    eo["hex_id"] = hb;
    eo["group_id"] = ev.groupId;
    eo["remote_type"] = ev.remoteType;
    eo["action"] = ev.action;
    eo["gesture"] = ev.gesture;
    if (ev.segment > 0) {
      eo["segment"] = ev.segment;
      eo["segments_total"] = ev.segmentsTotal;
      eo["label"] = ev.label;
    }
    eo["value"] = ev.value;
    eo["v2_dec"] = ev.v2Dec;
    eo["source"] = ev.source;
  }
}

void RemoteRegistry::publishDiscovery() {
  if (!loaded) load();

  for (const auto& r : remotes) {
    char discTopic[96];
    snprintf(discTopic, sizeof(discTopic), "milight/remotes_registry/0x%04X", r.deviceId);

    DynamicJsonDocument doc(1536);
    JsonObject root = doc.to<JsonObject>();
    writeRemoteToJson(r, root, true);

    String out;
    serializeJson(doc, out);
    sendMqtt(discTopic, out.c_str(), true);
  }
}

const RegisteredRemote* RemoteRegistry::find(uint16_t deviceId) const {
  for (const auto& r : remotes) {
    if (r.active && r.deviceId == deviceId) {
      return &r;
    }
  }
  return nullptr;
}

uint8_t RemoteRegistry::resolveGroupId(uint16_t deviceId, uint8_t rawGroupId, uint8_t protocolMaxZones) {
  if (!loaded) load();
  uint8_t maxZ = (protocolMaxZones > 0) ? protocolMaxZones : 8;
  const RegisteredRemote* r = find(deviceId);
  if (r != nullptr && r->numZones > 0) {
    maxZ = r->numZones;
  }
  if (maxZ == 1) {
    lastActiveZoneByDev[deviceId] = 1;
    return 1;
  }
  if (rawGroupId <= maxZ) {
    lastActiveZoneByDev[deviceId] = rawGroupId;
    return rawGroupId;
  }
  auto it = lastActiveZoneByDev.find(deviceId);
  if (it != lastActiveZoneByDev.end()) {
    return it->second;
  }
  return 1;
}

bool RemoteRegistry::hasTypeOverride(uint16_t deviceId, MiLightRemoteType& outType) const {
  const RegisteredRemote* r = find(deviceId);
  if (r == nullptr) return false;
  outType = MiLightRemoteTypeHelpers::remoteTypeFromString(r->remoteType);
  return true;
}

bool RemoteRegistry::upsert(const RegisteredRemote& item) {
  if (item.deviceId == 0) return false;
  for (auto& r : remotes) {
    if (r.deviceId == item.deviceId) {
      r = item;
      save();
      return true;
    }
  }
  remotes.push_back(item);
  save();
  return true;
}

bool RemoteRegistry::remove(uint16_t deviceId) {
  for (auto it = remotes.begin(); it != remotes.end(); ++it) {
    if (it->deviceId == deviceId) {
      char discTopic[96];
      snprintf(discTopic, sizeof(discTopic), "milight/remotes_registry/0x%04X", deviceId);
      sendMqtt(discTopic, "", true);
      remotes.erase(it);
      save();
      return true;
    }
  }
  return false;
}

uint8_t RemoteRegistry::computeHueSegment(int32_t hueDeg, const SurfaceSegmentConfig& cfg, uint32_t stateKey) {
  uint8_t n = constrain(cfg.segments, (uint8_t)2, (uint8_t)12);
  float span = 360.0f / (float)n;
  float shifted = fmodf((float)hueDeg - (float)cfg.offsetDeg + 720.0f, 360.0f);
  uint8_t rawSeg = (uint8_t)constrain((int)floorf(shifted / span) + 1, 1, (int)n);

  // +/- 3.0 deg boundary hysteresis so resting near a pie boundary doesn't chatter
  auto prevIt = lastSegmentBySurface.find(stateKey);
  if (prevIt != lastSegmentBySurface.end() && prevIt->second >= 1 && prevIt->second <= n && prevIt->second != rawSeg) {
    float prevStart = (prevIt->second - 1) * span;
    float prevEnd = prevIt->second * span;
    float distStart = fabsf(shifted - prevStart);
    if (distStart > 180.0f) distStart = 360.0f - distStart;
    float distEnd = fabsf(shifted - prevEnd);
    if (distEnd > 180.0f) distEnd = 360.0f - distEnd;
    if (distStart < 3.0f || distEnd < 3.0f) {
      return prevIt->second;
    }
  }
  lastSegmentBySurface[stateKey] = rawSeg;
  return rawSeg;
}

uint8_t RemoteRegistry::computeLinearSegment(int32_t normPct, const SurfaceSegmentConfig& cfg, uint32_t stateKey) {
  uint8_t n = constrain(cfg.segments, (uint8_t)2, (uint8_t)6);
  float clamped = constrain((float)normPct, 0.0f, 100.0f);
  float span = 100.0f / (float)n;
  int idx = (int)floorf(clamped / span) + 1;
  uint8_t rawSeg = (uint8_t)constrain(idx, 1, (int)n);

  // +/- 2.0% boundary hysteresis on touch bars
  auto prevIt = lastSegmentBySurface.find(stateKey);
  if (prevIt != lastSegmentBySurface.end() && prevIt->second >= 1 && prevIt->second <= n && prevIt->second != rawSeg) {
    float boundary = (rawSeg > prevIt->second) ? (prevIt->second * span) : ((prevIt->second - 1) * span);
    if (fabsf(clamped - boundary) < 2.0f) {
      return prevIt->second;
    }
  }
  lastSegmentBySurface[stateKey] = rawSeg;
  return rawSeg;
}

bool RemoteRegistry::isRemoteLocked(uint16_t deviceId) const {
  const RegisteredRemote* r = find(deviceId);
  if (r == nullptr || !r->childLock.enabled) return false;
  auto it = childLockStates.find(deviceId);
  if (it == childLockStates.end() || it->second.lastActivityMs == 0) return true;
  uint32_t elapsedMs = millis() - it->second.lastActivityMs;
  return elapsedMs > ((uint32_t)r->childLock.idleTimeoutSec * 1000UL);
}

void RemoteRegistry::setRemoteLockState(uint16_t deviceId, bool locked) {
  ChildLockRuntime& rt = childLockStates[deviceId];
  rt.progress = 0;
  rt.lastStepMs = 0;
  rt.lastActivityMs = locked ? 0 : (millis() ? millis() : 1);
}

static bool matchesUnlockStep(const String& seqName, uint8_t stepIdx, uint8_t groupId, const String& action) {
  if (seqName == "z1_z2_z3") {
    uint8_t expectedZone = stepIdx + 1;
    return (groupId == expectedZone && action == "on");
  }
  if (seqName == "off_off_on") {
    if (stepIdx == 0 || stepIdx == 1) return (action == "off");
    return (action == "on");
  }
  // Default "on_off_on": ON -> OFF -> ON on any zone
  if (stepIdx == 0) return (action == "on");
  if (stepIdx == 1) return (action == "off");
  return (action == "on");
}

void RemoteRegistry::handlePacket(uint16_t deviceId, uint8_t groupId, const String& remoteTypeName, const JsonObject& result, bool fromVirtual) {
  if (!loaded) load();

  String action = "button_press";
  int32_t value = -1;

  if (result.containsKey("command")) {
    action = result["command"].as<const char*>();
  } else if (result.containsKey("brightness")) {
    action = "brightness";
    value = fromVirtual ? (int32_t)(result["brightness"] | 0) : max((int32_t)26, (int32_t)(result["brightness"] | 26));
  } else if (result.containsKey("level")) {
    action = "brightness";
    value = result["level"] | 0;
  } else if (result.containsKey("hue")) {
    action = "hue";
    value = result["hue"] | 0;
  } else if (result.containsKey("saturation")) {
    action = "saturation";
    value = result["saturation"] | 0;
  } else if (result.containsKey("color_temp")) {
    action = "color_temp";
    value = result["color_temp"] | 0;
  } else if (result.containsKey("kelvin")) {
    action = "kelvin";
    value = result["kelvin"] | 0;
  } else if (result.containsKey("mode")) {
    action = "mode";
    value = result["mode"] | 0;
  } else if (result.containsKey("state")) {
    const char* st = result["state"] | "OFF";
    action = (strcmp(st, "ON") == 0) ? "on" : "off";
    value = (strcmp(st, "ON") == 0) ? 1 : 0;
  } else if (result.containsKey("button_id")) {
    uint8_t bid = result["button_id"] | 0;
    uint8_t barg = result["argument"] | 0;
    if ((bid & 0x7F) == 0x03) {
      action = "delay_off_60s";
      value = barg;
    } else {
      action = "raw_btn_" + String(bid, HEX);
      value = barg;
    }
  }

  uint32_t actHash = 2166136261u;
  for (size_t i = 0; i < action.length(); i++) {
    actHash ^= (uint8_t)action[i];
    actHash *= 16777619u;
  }
  uint32_t now = millis();
  uint32_t sig = ((uint32_t)deviceId << 16) ^ ((uint32_t)groupId << 8) ^ actHash ^ ((uint32_t)(value & 0xFF) << 1);
  bool isSlider = (action == "hue" || action == "brightness" || action == "saturation" || action == "color_temp" || action == "kelvin");
  // 95ms single-burst RF dedup allows rapid human double/triple taps (140ms-360ms apart) right through!
  uint32_t dedupWindow = isSlider ? 65 : 95;
  if (!fromVirtual && sig == lastDedupSig && (now - lastDedupMs < dedupWindow)) {
    return;
  }
  lastDedupSig = sig;
  lastDedupMs = now;

  String v2DecStr = result["v2_dec"] | "";
  const RegisteredRemote* r = find(deviceId);

  // Unregistered remotes still update lastSeen immediately so Auto-Detect & Live Sniffer see them
  if (r == nullptr) {
    emitResolvedEvent(nullptr, deviceId, groupId, remoteTypeName, action, "single", value, fromVirtual, "", 0, 0, "", v2DecStr);
    return;
  }

  // Firmware Idle-Timeout Child / Safety Lock Gatekeeper (applies to physical 2.4GHz RF presses)
  if (!fromVirtual && r->childLock.enabled) {
    ChildLockRuntime& clRt = childLockStates[deviceId];
    bool currentlyLocked = (clRt.lastActivityMs == 0) || ((now - clRt.lastActivityMs) > ((uint32_t)r->childLock.idleTimeoutSec * 1000UL));
    if (currentlyLocked) {
      clRt.lastActivityMs = 0; // Ensure latched locked
      if (isSlider) {
        // Ignore continuous wheel/slider brushes silently while locked
        return;
      }
      if (clRt.progress > 0 && (now - clRt.lastStepMs > 6000UL)) {
        clRt.progress = 0; // Reset partial combo if >6s elapsed between taps
      }
      if (matchesUnlockStep(r->childLock.sequence, clRt.progress, groupId, action)) {
        clRt.progress++;
        clRt.lastStepMs = now;
        if (clRt.progress >= 3) {
          clRt.progress = 0;
          clRt.lastActivityMs = now ? now : 1;
          // Emit non-MQTT UI notification via lastSeen ring so UI shows instant Unlock!
          emitResolvedEvent(nullptr, deviceId, groupId, remoteTypeName, "child_lock_unlocked", "unlocked", 3, false, "", 3, 3, "Unlocked!", v2DecStr);
          return;
        } else {
          String progLbl = String("Child Lock ") + String(clRt.progress) + "/3";
          emitResolvedEvent(nullptr, deviceId, groupId, remoteTypeName, action, "locked", clRt.progress, false, "", clRt.progress, 3, progLbl, v2DecStr);
          return;
        }
      } else {
        // Check if this tap starts step 1 fresh
        if (matchesUnlockStep(r->childLock.sequence, 0, groupId, action)) {
          clRt.progress = 1;
          clRt.lastStepMs = now;
          emitResolvedEvent(nullptr, deviceId, groupId, remoteTypeName, action, "locked", 1, false, "", 1, 3, "Child Lock 1/3", v2DecStr);
        } else {
          clRt.progress = 0;
          emitResolvedEvent(nullptr, deviceId, groupId, remoteTypeName, action, "locked", 0, false, "", 0, 3, "Locked (Enter 3-Tap Code)", v2DecStr);
        }
        return;
      }
    } else {
      // Remote is currently unlocked — refresh idle countdown timer on every physical press!
      clRt.lastActivityMs = now ? now : 1;
    }
  }

  // Check if this zone matches a Segmented Ring / Slider Macro-Pad surface!
  uint16_t zBit = (groupId < 16) ? (uint16_t)(1u << groupId) : 0x8000;
  auto zoneAllowed = [&](const SurfaceSegmentConfig& sc) -> bool {
    return sc.enabled && ((sc.zoneMask & zBit) != 0);
  };

  if (action == "hue" && zoneAllowed(r->hueRing)) {
    uint32_t sk = ((uint32_t)deviceId << 16) | ((uint32_t)groupId << 4) | 1u;
    uint8_t seg = computeHueSegment(value, r->hueRing, sk);
    String lbl = (seg >= 1 && seg <= 12) ? r->hueRing.labels[seg - 1] : "";
    if (lbl.length() == 0) lbl = "Wheel Slice " + String(seg);
    if (fromVirtual) {
      emitResolvedEvent(r, deviceId, groupId, remoteTypeName, "hue_seg_" + String(seg), "single", value, true, "hue_ring", seg, r->hueRing.segments, lbl, v2DecStr);
    } else {
      PendingSegmentSettle& ps = segSettleTrackers[sk];
      ps.active = true; ps.deviceId = deviceId; ps.groupId = groupId; ps.remoteType = remoteTypeName;
      ps.surface = "hue_ring"; ps.segment = seg; ps.segmentsTotal = r->hueRing.segments;
      ps.label = lbl; ps.rawValue = value; ps.fromVirtual = false; ps.settleDeadlineMs = now + 140;
    }
    return;
  }

  if ((action == "color_temp" || action == "kelvin") && zoneAllowed(r->cctBar)) {
    uint32_t sk = ((uint32_t)deviceId << 16) | ((uint32_t)groupId << 4) | 2u;
    int32_t pct = (value > 100) ? constrain((int)roundf(((370.0f - value) / (370.0f - 153.0f)) * 100.0f), 0, 100) : constrain((int)value, 0, 100);
    uint8_t seg = computeLinearSegment(pct, r->cctBar, sk);
    String lbl = (seg >= 1 && seg <= 12) ? r->cctBar.labels[seg - 1] : "";
    if (lbl.length() == 0) lbl = "CCT Slot " + String(seg);
    if (fromVirtual) {
      emitResolvedEvent(r, deviceId, groupId, remoteTypeName, "cct_seg_" + String(seg), "single", pct, true, "cct_bar", seg, r->cctBar.segments, lbl, v2DecStr);
    } else {
      PendingSegmentSettle& ps = segSettleTrackers[sk];
      ps.active = true; ps.deviceId = deviceId; ps.groupId = groupId; ps.remoteType = remoteTypeName;
      ps.surface = "cct_bar"; ps.segment = seg; ps.segmentsTotal = r->cctBar.segments;
      ps.label = lbl; ps.rawValue = pct; ps.fromVirtual = false; ps.settleDeadlineMs = now + 140;
    }
    return;
  }

  if (action == "saturation" && zoneAllowed(r->satBar)) {
    uint32_t sk = ((uint32_t)deviceId << 16) | ((uint32_t)groupId << 4) | 3u;
    int32_t pct = constrain((int)value, 0, 100);
    uint8_t seg = computeLinearSegment(pct, r->satBar, sk);
    String lbl = (seg >= 1 && seg <= 12) ? r->satBar.labels[seg - 1] : "";
    if (lbl.length() == 0) lbl = "Sat Slot " + String(seg);
    if (fromVirtual) {
      emitResolvedEvent(r, deviceId, groupId, remoteTypeName, "sat_seg_" + String(seg), "single", pct, true, "sat_bar", seg, r->satBar.segments, lbl, v2DecStr);
    } else {
      PendingSegmentSettle& ps = segSettleTrackers[sk];
      ps.active = true; ps.deviceId = deviceId; ps.groupId = groupId; ps.remoteType = remoteTypeName;
      ps.surface = "sat_bar"; ps.segment = seg; ps.segmentsTotal = r->satBar.segments;
      ps.label = lbl; ps.rawValue = pct; ps.fromVirtual = false; ps.settleDeadlineMs = now + 140;
    }
    return;
  }

  if (action == "brightness" && zoneAllowed(r->briBar)) {
    uint32_t sk = ((uint32_t)deviceId << 16) | ((uint32_t)groupId << 4) | 4u;
    int32_t pct = fromVirtual ? constrain((int)value, 0, 100) : constrain((int)roundf(((value - 61.0f) / (255.0f - 61.0f)) * 100.0f), 0, 100);
    uint8_t seg = computeLinearSegment(pct, r->briBar, sk);
    String lbl = (seg >= 1 && seg <= 12) ? r->briBar.labels[seg - 1] : "";
    if (lbl.length() == 0) lbl = "Bri Slot " + String(seg);
    if (fromVirtual) {
      emitResolvedEvent(r, deviceId, groupId, remoteTypeName, "bri_seg_" + String(seg), "single", pct, true, "bri_bar", seg, r->briBar.segments, lbl, v2DecStr);
    } else {
      PendingSegmentSettle& ps = segSettleTrackers[sk];
      ps.active = true; ps.deviceId = deviceId; ps.groupId = groupId; ps.remoteType = remoteTypeName;
      ps.surface = "bri_bar"; ps.segment = seg; ps.segmentsTotal = r->briBar.segments;
      ps.label = lbl; ps.rawValue = pct; ps.fromVirtual = false; ps.settleDeadlineMs = now + 140;
    }
    return;
  }

  // Hardware Long-Press (night_mode = Hold OFF)
  if (action == "night_mode") {
    uint32_t offKey = ((uint32_t)deviceId << 16) | ((uint32_t)groupId << 8) | 0x02u;
    tapTrackers.erase(offKey);
    if (!r->gestures.longEnabled) return;
    emitResolvedEvent(r, deviceId, groupId, remoteTypeName, "night_mode", "long", value, fromVirtual, "", 0, 0, "", v2DecStr);
    return;
  }

  // Discrete Button Multi-Tap Classifier (Zero-latency fast path if double & triple are disabled!)
  bool tapEligible = (action == "on" || action == "off" || action == "mode_speed_up" || action == "mode_speed_down" || action == "set_white");
  if (tapEligible) {
    bool multiEnabled = (r->gestures.doubleEnabled || r->gestures.tripleEnabled);
    if (!multiEnabled) {
      if (!r->gestures.shortEnabled) return;
      emitResolvedEvent(r, deviceId, groupId, remoteTypeName, action, "single", value, fromVirtual, "", 0, 0, "", v2DecStr);
      return;
    }

    uint8_t btnCode = (action == "on") ? 1 : (action == "off") ? 2 : (action == "mode_speed_up") ? 3 : (action == "mode_speed_down") ? 4 : 5;
    uint32_t tKey = ((uint32_t)deviceId << 16) | ((uint32_t)groupId << 8) | btnCode;
    PendingTapTracker& tr = tapTrackers[tKey];
    if (tr.active) {
      tr.count = min((uint8_t)3, (uint8_t)(tr.count + 1));
      tr.lastTapMs = now;
      tr.fromVirtual = fromVirtual;
      uint8_t maxTarget = r->gestures.tripleEnabled ? 3 : 2;
      if (tr.count >= maxTarget) {
        PendingTapTracker done = tr;
        tapTrackers.erase(tKey);
        emitResolvedEvent(r, deviceId, groupId, remoteTypeName, action, (done.count >= 3) ? "triple" : "double", value, fromVirtual, "", 0, 0, "", v2DecStr);
        return;
      }
      tr.deadlineMs = now + r->gestures.multiTapWindowMs;
    } else {
      tr.active = true;
      tr.deviceId = deviceId;
      tr.groupId = groupId;
      tr.remoteType = remoteTypeName;
      tr.baseAction = action;
      tr.value = value;
      tr.fromVirtual = fromVirtual;
      tr.count = 1;
      tr.lastTapMs = now;
      tr.deadlineMs = now + r->gestures.multiTapWindowMs;
    }
    return;
  }

  // Continuous analog sliders/wheels execute immediately
  emitResolvedEvent(r, deviceId, groupId, remoteTypeName, action, "single", value, fromVirtual, "", 0, 0, "", v2DecStr);
}

void RemoteRegistry::emitResolvedEvent(
  const RegisteredRemote* r,
  uint16_t deviceId,
  uint8_t groupId,
  const String& remoteTypeName,
  const String& action,
  const String& gesture,
  int32_t value,
  bool fromVirtual,
  const String& surface,
  uint8_t segment,
  uint8_t segmentsTotal,
  const String& segLabel,
  const String& v2Dec
) {
  uint32_t now = millis();
  lastSeen.seq++;
  lastSeen.timestampMs = now;
  lastSeen.deviceId = deviceId;
  lastSeen.groupId = groupId;
  lastSeen.remoteType = remoteTypeName;
  lastSeen.action = action;
  lastSeen.gesture = gesture;
  lastSeen.segment = segment;
  lastSeen.segmentsTotal = segmentsTotal;
  lastSeen.label = segLabel;
  lastSeen.value = value;
  lastSeen.v2Dec = v2Dec;
  lastSeen.source = fromVirtual ? "virtual" : "rf";

  recentRing[ringHead] = lastSeen;
  ringHead = (ringHead + 1) % RING_CAP;
  if (ringCount < RING_CAP) ringCount++;

  if (r == nullptr) return;

  // Update merged per-zone state in RAM
  uint32_t zKey = ((uint32_t)deviceId << 8) | groupId;
  AccumulatedZoneState& zs = zoneStates[zKey];
  zs.lastAction = action;
  if (action == "off" || action == "night_mode") {
    zs.on = false;
  } else if (action == "on" || action == "brightness" || action == "hue" || action == "saturation" || action == "color_temp" || action == "kelvin" || action == "set_white") {
    zs.on = true;
  }
  if (action == "brightness") zs.brightness = (int16_t)value;
  if (action == "hue")        { zs.hue = (int16_t)value; zs.colorMode = "color"; }
  if (action == "saturation") { zs.saturation = (int16_t)value; zs.colorMode = "color"; }
  if (action == "color_temp" || action == "kelvin") { zs.colorTemp = (int16_t)value; zs.colorMode = "white"; }
  if (action == "set_white")  { zs.colorMode = "white"; }
  if (action == "mode")       { zs.mode = (int16_t)value; zs.colorMode = "scene"; }

  char hexBuf[10];
  snprintf(hexBuf, sizeof(hexBuf), "0x%04X", deviceId);
  String gestureAction = (gesture == "double" || gesture == "triple") ? (action + "_" + gesture) : action;

  // 1. Instant Stateless Event Topic (/event, retain=false)
  StaticJsonDocument<384> evDoc;
  evDoc["hex_id"] = hexBuf;
  evDoc["group_id"] = groupId;
  evDoc["model"] = r->visualModel;
  evDoc["action"] = action;
  evDoc["gesture"] = gesture;
  evDoc["gesture_action"] = gestureAction;
  if (segment > 0) {
    evDoc["surface"] = surface;
    evDoc["segment"] = segment;
    evDoc["segments_total"] = segmentsTotal;
    if (segLabel.length() > 0) evDoc["label"] = segLabel;
  }
  if (value >= 0) evDoc["value"] = value;
  if (fromVirtual) evDoc["source"] = "virtual";

  char evPayload[384];
  serializeJson(evDoc, evPayload, sizeof(evPayload));
  char eventTopic[128];
  snprintf(eventTopic, sizeof(eventTopic), "%s/zone/%u/event", r->topicPrefix.c_str(), groupId);
  sendMqtt(eventTopic, evPayload, false);

  // 2. Full Merged Zone State Topic (/state, retain=true)
  StaticJsonDocument<256> stDoc;
  stDoc["on"] = zs.on;
  stDoc["mode"] = zs.colorMode;
  stDoc["last"] = gestureAction;
  if (zs.brightness >= 0) stDoc["bri"] = zs.brightness;
  if (zs.hue >= 0)        stDoc["hue"] = zs.hue;
  if (zs.saturation >= 0) stDoc["sat"] = zs.saturation;
  if (zs.colorTemp >= 0)  stDoc["ct"] = zs.colorTemp;
  if (zs.mode >= 0)       stDoc["fx"] = zs.mode;

  char stPayload[256];
  serializeJson(stDoc, stPayload, sizeof(stPayload));
  char stateTopic[128];
  snprintf(stateTopic, sizeof(stateTopic), "%s/zone/%u/state", r->topicPrefix.c_str(), groupId);
  sendMqtt(stateTopic, stPayload, true);
}

bool RemoteRegistry::virtualSend(uint16_t deviceId, const String& remoteTypeName, uint8_t groupId, const String& action, int32_t value) {
  // Support direct segmented virtual clicks (e.g., hue_seg_2, cct_seg_3) without transmitting raw bulb RF if desired
  if (action.startsWith("hue_seg_") || action.startsWith("cct_seg_") || action.startsWith("sat_seg_") || action.startsWith("bri_seg_")) {
    const RegisteredRemote* r = find(deviceId);
    String surf = action.startsWith("hue_") ? "hue_ring" : action.startsWith("cct_") ? "cct_bar" : action.startsWith("sat_") ? "sat_bar" : "bri_bar";
    uint8_t seg = (uint8_t)constrain(action.substring(8).toInt(), 1, 12);
    uint8_t total = 4;
    String lbl = action;
    if (r != nullptr) {
      const SurfaceSegmentConfig& sc = (surf == "hue_ring") ? r->hueRing : (surf == "cct_bar") ? r->cctBar : (surf == "sat_bar") ? r->satBar : r->briBar;
      total = sc.segments;
      if (seg >= 1 && seg <= 12 && sc.labels[seg - 1].length() > 0) lbl = sc.labels[seg - 1];
    }
    emitResolvedEvent(r, deviceId, groupId, remoteTypeName, action, "single", value, true, surf, seg, total, lbl);
    return true;
  }

  StaticJsonDocument<192> fakeDoc;
  JsonObject fakeObj = fakeDoc.to<JsonObject>();

  // Only transmit RF to physical Mi-Light bulbs if this surface is NOT hijacked as a Segmented Macro-Pad on this zone!
  const RegisteredRemote* r = find(deviceId);
  uint16_t zBit = (groupId < 16) ? (uint16_t)(1u << groupId) : 0x8000;
  bool segmentedOverride = false;
  if (r != nullptr) {
    if (action == "hue" && r->hueRing.enabled && (r->hueRing.zoneMask & zBit)) segmentedOverride = true;
    if ((action == "kelvin" || action == "color_temp") && r->cctBar.enabled && (r->cctBar.zoneMask & zBit)) segmentedOverride = true;
    if (action == "saturation" && r->satBar.enabled && (r->satBar.zoneMask & zBit)) segmentedOverride = true;
    if (action == "brightness" && r->briBar.enabled && (r->briBar.zoneMask & zBit)) segmentedOverride = true;
  }

  if (!segmentedOverride && milightClientPtr != nullptr && *milightClientPtr != nullptr) {
    MiLightClient* mc = *milightClientPtr;
    mc->setInternalOrigin(true);
    MiLightRemoteType rType = MiLightRemoteTypeHelpers::remoteTypeFromString(remoteTypeName);
    mc->prepare(rType, deviceId, groupId);

    if (action == "on") {
      mc->updateStatus(MiLightStatus::ON);
      fakeObj["state"] = "ON";
    } else if (action == "off") {
      mc->updateStatus(MiLightStatus::OFF);
      fakeObj["state"] = "OFF";
    } else if (action == "set_white" || action == "white") {
      mc->updateColorWhite();
      fakeObj["command"] = "set_white";
    } else if (action == "night_mode") {
      mc->enableNightMode();
      fakeObj["command"] = "night_mode";
    } else if (action == "brightness") {
      uint8_t pct = (uint8_t)constrain(value, 0, 100);
      if (pct > 0) mc->updateBrightness(pct);
      fakeObj["brightness"] = pct;
    } else if (action == "hue") {
      uint16_t h = (uint16_t)((value % 360 + 360) % 360);
      mc->updateHue(h);
      fakeObj["hue"] = h;
    } else if (action == "kelvin" || action == "color_temp") {
      uint8_t k = (uint8_t)constrain(value, 0, 100);
      mc->updateTemperature(k);
      fakeObj["color_temp"] = (uint16_t)roundf(370.0f - (k / 100.0f) * (370.0f - 153.0f));
    } else if (action == "saturation") {
      uint8_t s = (uint8_t)constrain(value, 0, 100);
      mc->updateSaturation(s);
      fakeObj["saturation"] = s;
    } else if (action == "mode") {
      uint8_t m = (uint8_t)constrain(value, 0, 8);
      mc->updateMode(m);
      fakeObj["mode"] = m;
    } else if (action == "mode_speed_up") {
      mc->modeSpeedUp();
      fakeObj["command"] = "mode_speed_up";
    } else if (action == "mode_speed_down") {
      mc->modeSpeedDown();
      fakeObj["command"] = "mode_speed_down";
    }
    mc->setInternalOrigin(false);
  } else {
    if (action == "hue") fakeObj["hue"] = (value % 360 + 360) % 360;
    else if (action == "kelvin" || action == "color_temp") fakeObj["kelvin"] = constrain((int)value, 0, 100);
    else if (action == "saturation") fakeObj["saturation"] = constrain((int)value, 0, 100);
    else if (action == "brightness") fakeObj["brightness"] = constrain((int)value, 0, 100);
    else if (action == "on") fakeObj["state"] = "ON";
    else if (action == "off") fakeObj["state"] = "OFF";
    else fakeObj["command"] = action;
  }

  handlePacket(deviceId, groupId, remoteTypeName, fakeObj, true);
  return true;
}
