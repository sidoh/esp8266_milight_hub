#include <MiLightClient.h>
#include <Settings.h>
#include <AsyncMqttClient.h>
#include <WiFiClient.h>
#include <MiLightRadioConfig.h>
#include <atomic>

#ifndef MQTT_CONNECTION_ATTEMPT_FREQUENCY
#define MQTT_CONNECTION_ATTEMPT_FREQUENCY 5000
#endif

#ifndef _MQTT_CLIENT_H
#define _MQTT_CLIENT_H

#ifndef MQTTBUFFER
#define MQTTBUFFER 400
#endif

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
  AsyncMqttClient mqttClient;
  MiLightClient*& milightClient;
  Settings& settings;
  char* domain;
  unsigned long lastConnectAttempt;
  std::atomic<bool> mqttmsg;
  char mqtttopic[MQTTBUFFER];
  char mqttpayload[MQTTBUFFER];

  void connect();
  void subscribe();
  void publishCallback(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t  length, size_t index, size_t total);
  void publishLoop(void);
  void connectCallback(bool sessionPresent);
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
