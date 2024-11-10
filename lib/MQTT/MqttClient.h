#include <MiLightClient.h>
#include <Settings.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <MiLightRadioConfig.h>
#include <ESPId.h>
#include <map>
#include <pgmspace.h>

#ifndef MQTT_CONNECTION_ATTEMPT_FREQUENCY
#define MQTT_CONNECTION_ATTEMPT_FREQUENCY 5000
#endif

#ifndef MQTT_PACKET_CHUNK_SIZE
#define MQTT_PACKET_CHUNK_SIZE 128
#endif

#ifndef _MQTT_CLIENT_H
#define _MQTT_CLIENT_H

static const std::map<int, const __FlashStringHelper*> MQTT_STATUS_STRINGS = {
  {MQTT_CONNECTION_TIMEOUT, FPSTR("Connection Timeout")},
  {MQTT_CONNECTION_LOST, FPSTR("Connection Lost")},
  {MQTT_CONNECT_FAILED, FPSTR("Connect Failed")},
  {MQTT_DISCONNECTED, FPSTR("Disconnected")},
  {MQTT_CONNECTED, FPSTR("Connected")},
  {MQTT_CONNECT_BAD_PROTOCOL, FPSTR("Connect Bad Protocol")},
  {MQTT_CONNECT_BAD_CLIENT_ID, FPSTR("Connect Bad Client ID")},
  {MQTT_CONNECT_UNAVAILABLE, FPSTR("Connect Unavailable")},
  {MQTT_CONNECT_BAD_CREDENTIALS, FPSTR("Connect Bad Credentials")},
  {MQTT_CONNECT_UNAUTHORIZED, FPSTR("Connect Unauthorized")}
};

enum MqttConnectionStatus : int {
  ConnectionTimeout     = MQTT_CONNECTION_TIMEOUT,
  ConnectionLost        = MQTT_CONNECTION_LOST,
  ConnectFailed         = MQTT_CONNECT_FAILED,
  Disconnected          = MQTT_DISCONNECTED,
  Connected             = MQTT_CONNECTED,
  ConnectBadProtocol    = MQTT_CONNECT_BAD_PROTOCOL,
  ConnectBadClientId    = MQTT_CONNECT_BAD_CLIENT_ID,
  ConnectUnavailable    = MQTT_CONNECT_UNAVAILABLE,
  ConnectBadCredentials = MQTT_CONNECT_BAD_CREDENTIALS,
  ConnectUnauthorized   = MQTT_CONNECT_UNAUTHORIZED
};

class MqttClient {
public:
  using OnConnectFn = std::function<void()>;

  MqttClient(Settings& settings, MiLightClient*& milightClient);
  ~MqttClient();

  void begin();
  void handleClient();
  void reconnect();
  void sendUpdate(const MiLightRemoteConfig& remoteConfig, uint16_t deviceId, uint16_t groupId, const char* update);
  void sendState(const MiLightRemoteConfig& remoteConfig, uint16_t deviceId, uint16_t groupId, const char* update);
  void send(const char* topic, const char* message, const bool retain = false);
  void onConnect(OnConnectFn fn);
  bool isConnected();
  MqttConnectionStatus getConnectionStatus();
  const __FlashStringHelper* getConnectionStatusString();

  String bindTopicString(const String& topicPattern, const BulbId& bulbId);

private:
  WiFiClient tcpClient;
  PubSubClient mqttClient;
  MiLightClient*& milightClient;
  Settings& settings;
  char* domain;
  unsigned long lastConnectAttempt;
  OnConnectFn onConnectFn;
  bool connected;

  void sendBirthMessage();
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

  String generateConnectionStatusMessage(const char* status);
};

#endif
