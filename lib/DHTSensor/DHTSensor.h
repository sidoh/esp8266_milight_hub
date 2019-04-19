#include <Arduino.h>
#include "DHT.h"

#ifndef _DHT_SENSOR_H
#define _DHT_SENSOR_H

class DHTSensor {
  public:

    DHTSensor(int8_t dhtPin, int8_t dhtType=DHT::AUTO_DETECT, boolean inFahrenheit=false, uint updateInterval=5000);
    void changePin(int8_t dhtPin);
    void changeType(int8_t dhtType=DHT::AUTO_DETECT);
    void changePinAndType(int8_t dhtPin, int8_t dhtType=DHT::AUTO_DETECT);
    void setUpdateInterval(uint updateInterval=5000);
    void setTemperatureMode(boolean inFahrenheit=false);

    bool handle();

    float getHumidity();
    float getTemperature();

  private:
    void initDHTSensor();
    void setPin(int8_t dhtPin);
    void setType(int8_t dhtType);

    DHT dhtSensor;
    uint8_t _dhtPin;
    DHT::DHT_MODEL_t _dhtType;

    boolean _inFahrenheit;
    float _humidity;
    float _temperature;

    unsigned long _timer = 0;
    uint _updateInterval;
};

#endif