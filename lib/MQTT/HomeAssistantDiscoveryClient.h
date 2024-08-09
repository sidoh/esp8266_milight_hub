#pragma once

#include <BulbId.h>
#include <MqttClient.h>
#include <ESPId.h>
#include <map>

class HomeAssistantDiscoveryClient {
public:
  HomeAssistantDiscoveryClient(Settings& settings, MqttClient* mqttClient);

  virtual void addConfig(const char* alias, const BulbId& bulbId);
  virtual void removeConfig(const BulbId& bulbId);

  virtual void sendDiscoverableDevices(const std::map<String, GroupAlias>& aliases);
  virtual void removeOldDevices(const std::map<uint32_t, BulbId>& aliases);

protected:
  Settings& settings;
  MqttClient* mqttClient;

  String buildTopic(const BulbId& bulbId);
  String bindTopicVariables(const String& topic, const char* alias, const BulbId& bulbId);
  void addNumberedEffects(JsonArray& effectList, uint8_t start, uint8_t end);
};