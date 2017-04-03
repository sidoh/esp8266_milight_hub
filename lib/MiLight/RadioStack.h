#include <RF24.h>
#include <PL1167_nRF24.h>
#include <MiLightRadioConfig.h>
#include <MiLightRadio.h>
#include <MiLightRadioPL1167_LT8900.h>
#include <MiLightRadioInterface.h>
#include <Settings.h>

#ifndef _RADIO_STACK_H
#define _RADIO_STACK_H

class RadioStack {
public:
  RadioStack(RF24& rf, const MiLightRadioConfig& config)
    : config(config),UsedInterfaceType(nRF24)
  {
    nrf = new PL1167_nRF24(rf);
    radio = new MiLightRadio(*nrf, config);
  }

  RadioStack(byte byCSPin, byte byResetPin, byte byPktFlag, const MiLightRadioConfig& config)
      : config(config),UsedInterfaceType(LT1167_PL8900)
    {
      radioPL1167_LT8900 = new MiLightRadioPL1167_LT8900(byCSPin, byResetPin, byPktFlag, config);
    }

  ~RadioStack() {
    delete radio;
    delete nrf;
    delete radioPL1167_LT8900;

  }

  inline MiLightRadioInterface* getRadioInterface()
  {
    if(UsedInterfaceType == nRF24)
    {
      return this->radio;
    }
    else if(UsedInterfaceType == LT1167_PL8900)
    {
      return this->radioPL1167_LT8900;
    }
  }

  const MiLightRadioConfig& config;

private:
  PL1167_nRF24 *nrf;
  MiLightRadio *radio;
  MiLightRadioPL1167_LT8900 *radioPL1167_LT8900;
  eRadioInterfaceType UsedInterfaceType;

};

#endif
