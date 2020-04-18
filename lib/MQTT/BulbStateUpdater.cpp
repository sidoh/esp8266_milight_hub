#include <BulbStateUpdater.h>

BulbStateUpdater::BulbStateUpdater(Settings& settings, MqttClient& mqttClient, GroupStateStore& stateStore)
  : settings(settings),
    mqttClient(mqttClient),
    stateStore(stateStore),
    lastFlush(0),
    lastQueue(0),
    enabled(true)
{ }

void BulbStateUpdater::enable() {
  this->enabled = true;
}

void BulbStateUpdater::disable() {
  this->enabled = false;
}

void BulbStateUpdater::enqueueUpdate(BulbId bulbId, GroupState& groupState) {
  staleGroups.push(bulbId);
  //Remember time, when queue was added for debounce delay
  lastQueue = millis();

}

void BulbStateUpdater::loop() {
  while (canFlush() && staleGroups.size() > 0) {
    BulbId bulbId = staleGroups.shift();
    GroupState* groupState = stateStore.get(bulbId);

    if (groupState->isMqttDirty()) {
      flushGroup(bulbId, *groupState);
      groupState->clearMqttDirty();
    }
  }
}

inline void BulbStateUpdater::flushGroup(BulbId bulbId, GroupState& state) {
  char buffer[200];
  StaticJsonDocument<200> json;
  JsonObject message = json.to<JsonObject>();

  state.applyState(message, bulbId, settings.groupStateFields);
  serializeJson(json, buffer);

  mqttClient.sendState(
    *MiLightRemoteConfig::fromType(bulbId.deviceType),
    bulbId.deviceId,
    bulbId.groupId,
    buffer
  );

  lastFlush = millis();
}

inline bool BulbStateUpdater::canFlush() const {
  return enabled && (millis() > (lastFlush + settings.mqttStateRateLimit) && millis() > (lastQueue + settings.mqttDebounceDelay));
}
