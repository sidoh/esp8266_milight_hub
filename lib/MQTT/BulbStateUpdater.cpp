#include <BulbStateUpdater.h>

BulbStateUpdater::BulbStateUpdater(Settings& settings, MqttClient& mqttClient, GroupStateStore& stateStore)
  : settings(settings),
    mqttClient(mqttClient),
    stateStore(stateStore),
    lastFlush(0)
{ }

void BulbStateUpdater::enqueueUpdate(GroupId groupId, GroupState& groupState) {
  // If can flush immediately, do so (avoids lookup of group state later).
  if (canFlush()) {
    flushGroup(groupId, groupState);
  } else {
    staleGroups.push(groupId);
  }
}

void BulbStateUpdater::loop() {
  while (canFlush() && staleGroups.size() > 0) {
    GroupId groupId = staleGroups.shift();
    GroupState& groupState = stateStore.get(groupId);

    if (groupState.isMqttDirty()) {
      flushGroup(groupId, groupState);
      groupState.clearMqttDirty();
    }
  }
}

inline void BulbStateUpdater::flushGroup(GroupId groupId, GroupState& state) {
  char buffer[200];
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& message = jsonBuffer.createObject();
  state.applyState(message);
  message.printTo(buffer);

  mqttClient.sendState(
    *MiLightRemoteConfig::fromType(groupId.deviceType),
    groupId.deviceId,
    groupId.groupId,
    buffer
  );

  lastFlush = millis();
}

inline bool BulbStateUpdater::canFlush() const {
  return (millis() > (lastFlush + settings.mqttStateRateLimit));
}
