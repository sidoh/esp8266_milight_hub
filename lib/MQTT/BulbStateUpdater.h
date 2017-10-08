/**
 * Enqueues updated bulb states and flushes them at the configured interval.
 */

#include <stddef.h>
#include <MqttClient.h>
#include <CircularBuffer.h>
#include <Settings.h>

#ifndef BULB_STATE_UPDATER
#define BULB_STATE_UPDATER

class BulbStateUpdater {
public:
  BulbStateUpdater(Settings& settings, MqttClient& mqttClient, GroupStateStore& stateStore);

  void enqueueUpdate(GroupId groupId, GroupState& groupState);
  void loop();

private:
  Settings& settings;
  MqttClient& mqttClient;
  GroupStateStore& stateStore;
  CircularBuffer<GroupId, MILIGHT_MAX_STALE_MQTT_GROUPS> staleGroups;
  unsigned long lastFlush;

  inline void flushGroup(GroupId groupId, GroupState& state);
  inline bool canFlush() const;
};

#endif
