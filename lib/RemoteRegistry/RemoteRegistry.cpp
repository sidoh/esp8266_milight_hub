#include <RemoteRegistry.h>
#include <FS.h>
#ifdef ESP32
  #include <SPIFFS.h>
  #endif
#include <ProjectFS.h>
extern bool initialized;

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
  lastSeen.seq = 0;
  lastSeen.timestampMs = 0;
  lastSeen.deviceId = 0;
  lastSeen.groupId = 0;
  lastSeen.remoteType = "";
  lastSeen.action = "";
  lastSeen.value = 0;
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
  if (discoverySent) return;
  uint32_t now = millis();
  if (now - lastDiscRetryMs < 3000) return;
  lastDiscRetryMs = now;

  bool hubReady = (mqttClientPtr != nullptr && *mqttClientPtr != nullptr && (*mqttClientPtr)->isConnected());
  if (hubReady) {
    publishDiscovery();
    discoverySent = true;
  } else if (fallbackPub) {
    // Test if fallback MQTT is connected by sending first remote or registry heartbeat
    if (fallbackPub("milight/remotes_registry/status", "online", true)) {
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

  DynamicJsonDocument doc(4096);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err || !doc.is<JsonArray>()) return;

  for (JsonObject obj : doc.as<JsonArray>()) {
    RegisteredRemote r;
    r.deviceId = obj["device_id"] | 0;
    r.remoteType = obj["remote_type"] | "fut089";
    r.visualModel = obj["visual_model"] | "FUT089";
    r.name = obj["name"] | "Remote";
    r.topicPrefix = obj["topic_prefix"] | "milight/actions/remote";
    r.numZones = obj["num_zones"] | 8;
    r.active = obj["active"] | true;
    if (r.deviceId != 0) {
      remotes.push_back(r);
    }
  }
}

void RemoteRegistry::save() {
  DynamicJsonDocument doc(4096);
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
    char hexBuf[10];
    snprintf(hexBuf, sizeof(hexBuf), "0x%04X", r.deviceId);
    obj["device_id"] = r.deviceId;
    obj["hex_id"] = hexBuf;
    obj["remote_type"] = r.remoteType;
    obj["visual_model"] = r.visualModel;
    obj["name"] = r.name;
    obj["topic_prefix"] = r.topicPrefix;
    obj["num_zones"] = r.numZones;
    obj["active"] = r.active;
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

    StaticJsonDocument<256> doc;
    char hexBuf[10];
    snprintf(hexBuf, sizeof(hexBuf), "0x%04X", r.deviceId);
    doc["id"] = r.deviceId;
    doc["hex"] = hexBuf;
    doc["type"] = r.remoteType;
    doc["model"] = r.visualModel;
    doc["name"] = r.name;
    doc["prefix"] = r.topicPrefix;
    doc["zones"] = r.numZones;
    doc["active"] = r.active;

    char buf[256];
    serializeJson(doc, buf, sizeof(buf));
    sendMqtt(discTopic, buf, true);
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

  // Single-zone remotes (FUT088, S2, B0, T0, FUT098, FUT020): map all slider/master presses cleanly to Zone 1
  if (maxZ == 1) {
    lastActiveZoneByDev[deviceId] = 1;
    return 1;
  }

  // Valid explicit zone (0..maxZ): remember it for subsequent slider packets
  if (rawGroupId <= maxZ) {
    lastActiveZoneByDev[deviceId] = rawGroupId;
    return rawGroupId;
  }

  // Wild V2 slider byte (> maxZ, e.g. Z83 / Z223 on rgb_cct or 0xE7 on fut089): return last selected zone (or 1)
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

void RemoteRegistry::handlePacket(uint16_t deviceId, uint8_t groupId, const String& remoteTypeName, const JsonObject& result, bool fromVirtual) {
  if (!loaded) load();

  String action = "button_press";
  int32_t value = -1;

  if (result.containsKey("command")) {
    action = result["command"].as<const char*>();
  } else if (result.containsKey("brightness")) {
    action = "brightness";
    value = max((int32_t)26, (int32_t)(result["brightness"] | 26));
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
    // MiBoxer B8 60S Delay OFF button sends command 0x03 (arg=0x01 or zone)
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
  uint32_t dedupWindow = isSlider ? 75 : 360;
  if (!fromVirtual && sig == lastDedupSig && (now - lastDedupMs < dedupWindow)) {
    if (!isSlider) lastDedupMs = now;
    return;
  }
  lastDedupSig = sig;
  lastDedupMs = now;

  lastSeen.seq++;
  lastSeen.timestampMs = now;
  lastSeen.deviceId = deviceId;
  lastSeen.groupId = groupId;
  lastSeen.remoteType = remoteTypeName;
  lastSeen.action = action;
  lastSeen.value = value;
  lastSeen.v2Dec = result["v2_dec"] | "";
  lastSeen.source = fromVirtual ? "virtual" : "rf";
  lastSeen.rawJson = "";
  serializeJson(result, lastSeen.rawJson);

  recentRing[ringHead] = lastSeen;
  ringHead = (ringHead + 1) % RING_CAP;
  if (ringCount < RING_CAP) ringCount++;

  const RegisteredRemote* r = find(deviceId);
  if (r == nullptr) {
    return;
  }

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

  // 1. Instant Stateless Event Topic (/event, retain=false)
  StaticJsonDocument<256> evDoc;
  evDoc["hex_id"] = hexBuf;
  evDoc["group_id"] = groupId;
  evDoc["model"] = r->visualModel;
  evDoc["action"] = action;
  if (value >= 0) evDoc["value"] = value;
  if (fromVirtual) evDoc["source"] = "virtual";

  char evPayload[256];
  serializeJson(evDoc, evPayload, sizeof(evPayload));
  char eventTopic[128];
  snprintf(eventTopic, sizeof(eventTopic), "%s/zone/%u/event", r->topicPrefix.c_str(), groupId);
  sendMqtt(eventTopic, evPayload, false);

  // 2. Full Merged Zone State Topic (/state, retain=true)
  StaticJsonDocument<256> stDoc;
  stDoc["on"] = zs.on;
  stDoc["mode"] = zs.colorMode;
  stDoc["last"] = zs.lastAction;
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
  StaticJsonDocument<192> fakeDoc;
  JsonObject fakeObj = fakeDoc.to<JsonObject>();

  if (milightClientPtr != nullptr && *milightClientPtr != nullptr) {
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
      uint8_t pct = (uint8_t)constrain(value, 10, 100);
      mc->updateBrightness(pct);
      fakeObj["brightness"] = (uint8_t)roundf((pct / 100.0f) * 255.0f);
    } else if (action == "hue") {
      uint16_t h = (uint16_t)(value % 360);
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
  }

  handlePacket(deviceId, groupId, remoteTypeName, fakeObj, true);
  return true;
}
