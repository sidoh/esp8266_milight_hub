/**
 * Enqueues updated bulb states and flushes them at the configured interval.
 */

#include <stddef.h>
#include <MqttClient.h>
#include <CircularBuffer.h>
#include <Settings.h>

#ifndef MILIGHT_MQTT_JSON_BUFFER_SIZE
#define MILIGHT_MQTT_JSON_BUFFER_SIZE 1024
#endif

#ifndef BULB_STATE_UPDATER
#define BULB_STATE_UPDATER

class BulbStateUpdater {
public:
  BulbStateUpdater(Settings& settings, MqttClient& mqttClient, GroupStateStore& stateStore);

  void enqueueUpdate(BulbId bulbId);
  void loop();
  void enable();
  void disable();

private:
  Settings& settings;
  MqttClient& mqttClient;
  GroupStateStore& stateStore;
  CircularBuffer<BulbId, MILIGHT_MAX_STALE_MQTT_GROUPS> staleGroups;
  unsigned long lastFlush;
  unsigned long lastQueue;
  bool enabled;

  inline void flushGroup(BulbId bulbId, GroupState& state);
  inline bool canFlush() const;
};

#endif
