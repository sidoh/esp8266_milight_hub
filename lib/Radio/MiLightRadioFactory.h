#include <RF24.h>
#include <PL1167_nRF24.h>
#include <MiLightRadioConfig.h>
#include <MiLightRadio.h>
#include <NRF24MiLightRadio.h>
#include <LT8900MiLightRadio.h>
#include <Settings.h>

#ifndef _MILIGHT_RADIO_FACTORY_H
#define _MILIGHT_RADIO_FACTORY_H

class MiLightRadioFactory {
public:

  virtual MiLightRadio* create(const MiLightRadioConfig& config) = 0;

  static MiLightRadioFactory* fromSettings(const Settings& settings);

};

class NRF24Factory : public MiLightRadioFactory {
public:

  NRF24Factory(uint8_t cePin, uint8_t csnPin);

  virtual MiLightRadio* create(const MiLightRadioConfig& config);

protected:

  RF24 rf24;

};

class LT8900Factory : public MiLightRadioFactory {
public:

  LT8900Factory(uint8_t csPin, uint8_t resetPin, uint8_t pktFlag);

  virtual MiLightRadio* create(const MiLightRadioConfig& config);

protected:

  uint8_t _csPin;
  uint8_t _resetPin;
  uint8_t _pktFlag;

};

#endif
