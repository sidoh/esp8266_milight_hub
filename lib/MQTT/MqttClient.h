#include <MiLightClient.h>
#include <Settings.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <MiLightRadioConfig.h>

#ifndef MQTT_CONNECTION_ATTEMPT_FREQUENCY
#define MQTT_CONNECTION_ATTEMPT_FREQUENCY 5000
#endif

#ifndef _MQTT_CLIENT_H
#define _MQTT_CLIENT_H

class MqttClient {
public:
  MqttClient(Settings& settings, MiLightClient*& milightClient);
  ~MqttClient();

  void begin();
  void handleClient();
  void reconnect();
  void sendUpdate(const MiLightRemoteConfig& remoteConfig, uint16_t deviceId, uint16_t groupId, const char* update);
  void sendState(const MiLightRemoteConfig& remoteConfig, uint16_t deviceId, uint16_t groupId, const char* update);

private:
  WiFiClient tcpClient;
  PubSubClient* mqttClient;
  MiLightClient*& milightClient;
  Settings& settings;
  char* domain;
  unsigned long lastConnectAttempt;

  bool connect();
  void subscribe();
  void publishCallback(char* topic, byte* payload, int length);
  void publish(
    const String& topic,
    const MiLightRemoteConfig& remoteConfig,
    uint16_t deviceId,
    uint16_t groupId,
    const char* update,
    const bool retain = false
  );

  inline static void bindTopicString(
    String& topicPattern,
    const MiLightRemoteConfig& remoteConfig,
    const uint16_t deviceId,
    const uint16_t groupId
  );
};

#endif
