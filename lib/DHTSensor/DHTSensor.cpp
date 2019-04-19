#include "DHTSensor.h"


DHTSensor::DHTSensor(int8_t dhtPin, int8_t dhtType, boolean inFahrenheit, uint updateInterval) {
  setPin(dhtPin);
  setType(dhtType);

  _inFahrenheit = inFahrenheit;
  _updateInterval = updateInterval;
  
  initDHTSensor();

  _timer = millis();
}

float DHTSensor::getHumidity() {
  return _humidity;
}

float DHTSensor::getTemperature() {
  if (_inFahrenheit)
      return dhtSensor.toFahrenheit(_temperature);
  else
      return _temperature;
}

void DHTSensor::initDHTSensor()
{
    dhtSensor.setup(_dhtPin, _dhtType);
}

// change DHT pin at runtime
void DHTSensor::changePin(int8_t dhtPin) {
  setPin(dhtPin);

  initDHTSensor();
}

// change DHT type at runtime
void DHTSensor::changeType(int8_t dhtType) {
  setType(dhtType);

  initDHTSensor();
}

// change DHT pin and type at runtime
void DHTSensor::changePinAndType(int8_t dhtPin, int8_t dhtType) {
  setPin(dhtPin);
  setType(dhtType);

  initDHTSensor();
}

void DHTSensor::setPin(int8_t dhtPin)
{
  if (dhtPin < 0) {
    _dhtPin = -1;
    return;
  }
  _dhtPin = dhtPin;

  pinMode(_dhtPin, INPUT);
}

void DHTSensor::setType(int8_t dhtType)
{
  if (dhtType < 0) {
    _dhtPin = -1;
    return;
  }
  _dhtType = (DHT::DHT_MODEL_t)dhtType;
}

// change how often the temperature/humidity is read from sensor
void DHTSensor::setUpdateInterval(uint updateInterval) {
  _updateInterval = updateInterval;
}

// change if temperature value is returned in C or F
void DHTSensor::setTemperatureMode(boolean inFahrenheit) {
  _inFahrenheit = inFahrenheit;
}

// call this function in your loop - it will return quickly after calculating if any changes need to 
// be made to the pin to flash the LED
bool DHTSensor::handle() {
  // is a pin defined?
  if (_dhtPin < 0) {
    return false;
  }

  if ((_timer + _updateInterval) < millis()) {
      _humidity = dhtSensor.getHumidity();
      _temperature = dhtSensor.getTemperature();
      return true;
  }
  return false;
}