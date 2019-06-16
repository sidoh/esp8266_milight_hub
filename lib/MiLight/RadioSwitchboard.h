#pragma once

#include <MiLightRadio.h>
#include <MiLightRemoteConfig.h>
#include <MiLightRadioConfig.h>
#include <MiLightRadioFactory.h>

class RadioSwitchboard {
public:
  RadioSwitchboard(
    std::shared_ptr<MiLightRadioFactory> radioFactory,
    GroupStateStore* stateStore,
    Settings& settings
  );

  std::shared_ptr<MiLightRadio> switchRadio(const MiLightRemoteConfig* remote);
  std::shared_ptr<MiLightRadio> switchRadio(size_t index);
  size_t getNumRadios() const;

  bool available();
  void write(uint8_t* packet, size_t length);
  size_t read(uint8_t* packet);

private:
  std::vector<std::shared_ptr<MiLightRadio>> radios;
  std::shared_ptr<MiLightRadio> currentRadio;
};