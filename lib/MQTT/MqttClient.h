#include <MiLightClient.h>
#include <Settings.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

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
};

#endif
