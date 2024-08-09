#pragma once

#include <BulbId.h>
#include <MqttClient.h>
#include <ESPId.h>
#include <map>

#include "HomeAssistantDiscoveryClient.h"

class HomeAssistantDiscoveryClientV2: public HomeAssistantDiscoveryClient {
public:
  HomeAssistantDiscoveryClientV2(Settings& settings, MqttClient* mqttClient);

  void addConfig(const char* alias, const BulbId& bulbId) override;
  //void removeConfig(const BulbId& bulbId) override;

  void sendDiscoverableDevices(const std::map<String, GroupAlias>& aliases) override;
  void removeOldDevices(const std::map<uint32_t, BulbId>& aliases) override;

private:

  void separateHassDevices(DynamicJsonDocument &config, const char* alias, const BulbId& bulbId);
  void addHub();
  String buildTopicForHub();
  String buildStateTopicForHub();
  String buildCommandTopicForHub();
};