#include <MiLightClient.h>
#include <Settings.h>
#include <AsyncMqttClient.h>
#include <MiLightRadioConfig.h>
#include <memory>

#ifndef MQTT_CONNECTION_ATTEMPT_FREQUENCY
#define MQTT_CONNECTION_ATTEMPT_FREQUENCY 5000
#endif

#ifndef MQTT_PACKET_CHUNK_SIZE
#define MQTT_PACKET_CHUNK_SIZE 128
#endif

#ifndef _MQTT_CLIENT_H
#define _MQTT_CLIENT_H

class MqttClient {
public:
  using OnConnectFn = std::function<void()>;

  MqttClient(Settings& settings, MiLightClient*& milightClient);
  ~MqttClient();

  void begin();
  void reconnect();
  void sendUpdate(const MiLightRemoteConfig& remoteConfig, uint16_t deviceId, uint16_t groupId, const char* update);
  void sendState(const MiLightRemoteConfig& remoteConfig, uint16_t deviceId, uint16_t groupId, const char* update);
  void send(const char* topic, const char* message, const bool retain = false);
  void onConnect(OnConnectFn fn);
  bool isConnected();

  String bindTopicString(const String& topicPattern, const BulbId& bulbId);

private:
  AsyncMqttClient mqttClient;
  MiLightClient*& milightClient;
  Settings& settings;
  std::shared_ptr<char> domain;
  unsigned long lastConnectAttempt;
  OnConnectFn onConnectFn;
  std::shared_ptr<char> lwtBuffer;

  void sendBirthMessage();
  void connect();
  void subscribe();
  void publishCallback(char* topic, char* payload, size_t length);
  void publish(
    const String& topic,
    const MiLightRemoteConfig& remoteConfig,
    uint16_t deviceId,
    uint16_t groupId,
    const char* update,
    const bool retain = false
  );

  String generateConnectionStatusMessage(const char* status);
};

#endif
