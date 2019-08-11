#pragma once

#include <BulbId.h>
#include <MqttClient.h>
#include <map>

class HomeAssistantDiscoveryClient {
public:
  HomeAssistantDiscoveryClient(Settings& settings, MqttClient* mqttClient);

  void addConfig(const char* alias, const BulbId& bulbId);
  void removeConfig(const BulbId& bulbId);

  void sendDiscoverableDevices(const std::map<String, BulbId>& aliases);
  void removeOldDevices(const std::map<uint32_t, BulbId>& aliases);

private:
  Settings& settings;
  MqttClient* mqttClient;

  String buildTopic(const BulbId& bulbId);
  String bindTopicVariables(const String& topic, const char* alias, const BulbId& bulbId);
  void addNumberedEffects(JsonArray& effectList, uint8_t start, uint8_t end);
};