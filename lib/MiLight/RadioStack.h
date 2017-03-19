#include <RF24.h>
#include <PL1167_nRF24.h>
#include <MiLightRadioConfig.h>
#include <MiLightRadio.h>

#ifndef _RADIO_STACK_H
#define _RADIO_STACK_H 

class RadioStack {
public:
  RadioStack(RF24& rf, MiLightRadioConfig& config) 
    : config(config)
  {
    nrf = new PL1167_nRF24(rf);
    radio = new MiLightRadio(*nrf, config);
  }
  
  ~RadioStack() {
    delete radio;
    delete nrf;
  }
  
  inline MiLightRadio* getRadio() {
    return this->radio;
  }
  
  MiLightRadioConfig& config;
  
private:
  PL1167_nRF24 *nrf;
  MiLightRadio *radio;
};

#endif