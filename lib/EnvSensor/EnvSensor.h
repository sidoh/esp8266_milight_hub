#include <Arduino.h>
#include "DHT.h"
#include <Wire.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_BME280.h"
#include "Sodaq_SHT2x.h"

#ifndef _ENV_SENSOR_H
#define _ENV_SENSOR_H

class EnvSensor {
  public:
    typedef enum {
      UNKNOWN,
      DHT11,
      DHT22,
      AM2302,  // Packaged DHT22
      RHT03,    // Equivalent to DHT22
      BME280,
      SHT21
    }
    SENSORS;

    EnvSensor(uint8_t sensorPin, SENSORS sensorType, uint8_t sensorAddr=0x76,boolean inFahrenheit=false, uint16_t updateInterval=5000);
    void changePin(uint8_t sensorPin);
    void changeType(SENSORS sensorType=SENSORS::UNKNOWN);
    void changePinAndType(uint8_t sensorPin, SENSORS sensorType=SENSORS::UNKNOWN);
    void changeAddr(uint8_t sensorAddr);
    void setUpdateInterval(uint updateInterval=5000);
    void setTemperatureMode(boolean inFahrenheit=false);
     
    bool handle();

    float getHumidity();
    float getTemperature();
    float getPressure();

  private:
    void initEnvSensor();
    void setPin(uint8_t sensorPin);
    void setType(SENSORS sensorType);
    void setAddress(uint8_t sensorAddr);

    DHT dhtSensor;
    Adafruit_BME280 bme;

    uint8_t _sensorPin;  //the sensors pin (used if not i2c)
    SENSORS _sensorType; //the type of sensor we're using
    uint8_t _sensorAddr; //i2c Address for relevant sensors

    boolean _inFahrenheit;
    float _humidity;
    float _temperature;
    float _pressure;

    unsigned long _timer = 0;
    uint _updateInterval;
};

#endif