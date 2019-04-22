#include <EnvSensor.h>

EnvSensor::EnvSensor(uint8_t sensorPin, SENSORS sensorType, uint8_t sensorAddr, boolean inFahrenheit, uint16_t updateInterval) {
  setPin(sensorPin);
  setType(sensorType);
  setAddress(sensorAddr);

  _inFahrenheit = inFahrenheit;
  _updateInterval = updateInterval;
  
  initEnvSensor();

  _timer = millis();
}

float EnvSensor::getHumidity() {
  return _humidity;
}

float EnvSensor::getTemperature() {
  if (_inFahrenheit)
      return (_temperature * 9.0 / 5.0) + 32.0;
  else
      return _temperature;
}

float EnvSensor::getPressure() {
  return _pressure;
}

void EnvSensor::initEnvSensor()
{
  _temperature = -1;
  _humidity = -1;
  _pressure = -1;
  switch(_sensorType)
  {
    case SENSORS::DHT11:
    case SENSORS::DHT22:
    case SENSORS::AM2302:
    case SENSORS::RHT03: dhtSensor.setup(_sensorPin, (DHTesp::DHT_MODEL_t)_sensorType); return;
    case SENSORS::BME280: Wire.begin(); bme.begin(_sensorAddr);
    case SENSORS::SHT21: Wire.begin();
    default: break;
  }
}

// change DHT pin at runtime
void EnvSensor::changePin(uint8_t sensorPin) {
  if (_sensorPin != sensorPin) {
    setPin(sensorPin);

    initEnvSensor();
  }
}

// change DHT type at runtime
void EnvSensor::changeType(SENSORS sensorType) {
  if (_sensorType != sensorType) {
    setType(sensorType);

    initEnvSensor();
  }
}

// change DHT pin and type at runtime
void EnvSensor::changePinAndType(uint8_t sensorPin, SENSORS sensorType) {
  bool changesMade = false;

  if (_sensorPin != sensorPin) {
    setPin(sensorPin);
    changesMade = true;
  }

  if (_sensorType != sensorType) {
    setType(sensorType);
    changesMade = true;
  }

  if (changesMade) {
    initEnvSensor();
  }
}
// change sensor i2c address
void EnvSensor::changeAddr(uint8_t sensorAddr) {
  if (_sensorAddr != sensorAddr) {
    setAddress(sensorAddr);

    initEnvSensor();
  }
}

void EnvSensor::setPin(uint8_t sensorPin)
{
  if (sensorPin < 0) {
    _sensorPin = -1;
    return;
  }
  _sensorPin = sensorPin;

//  pinMode(_sensorPin, INPUT);
}

void EnvSensor::setType(SENSORS sensorType)
{
  if (sensorType < 0) {
    _sensorType = SENSORS::UNKNOWN;
    return;
  }
  _sensorType = sensorType;
}

void EnvSensor::setAddress(uint8_t sensorAddr) {
  _sensorAddr = sensorAddr;
}

// change how often the temperature/humidity is read from sensor
void EnvSensor::setUpdateInterval(uint updateInterval) {
  if (updateInterval < 2000) {
    _updateInterval = 2000;
  }
  else {
    _updateInterval = updateInterval;
  }
}

// change if temperature value is returned in C or F
void EnvSensor::setTemperatureMode(boolean inFahrenheit) {
  _inFahrenheit = inFahrenheit;
}

// call this function in your loop - it will return quickly after calculating if any changes need to 
// be made to the pin to flash the LED
bool EnvSensor::handle() {
  // is a pin defined?
  if (_sensorPin < 0) {
    return false;
  }

  if ((_timer + _updateInterval) < millis()) {
    switch(_sensorType)
    {
      case SENSORS::DHT11:
      case SENSORS::DHT22:
      case SENSORS::AM2302:
      case SENSORS::RHT03: _humidity     = dhtSensor.getHumidity();
                           _temperature  = dhtSensor.getTemperature();
                           _pressure     = -1;
                           break;
      case SENSORS::BME280: _temperature = bme.readTemperature();
                            _humidity    = bme.readHumidity();
                            _pressure    = (int)(bme.readPressure() / 10) / 10.0; //convert Paskal to hPa with one decimal place
                            break;
      case SENSORS::SHT21: _temperature  = SHT2x.GetTemperature();
                           _humidity     = SHT2x.GetHumidity();
                           _pressure     = -1;
                           break;
      default: _temperature = -1; _humidity = -1; _pressure = -1; break;
    }

    _temperature = (int)(_temperature * 100) / 100.0;  //two decimal places max
    _humidity = (int)(_humidity * 100) / 100.0;        //two decimal places max

    _timer = millis();
    return true;
  }
  return false;
}