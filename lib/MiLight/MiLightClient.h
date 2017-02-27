#include <Arduino.h>
#include <MiLightRadio.h>
#include <PL1167_nRF24.h>
#include <RF24.h>
#include <MiLightButtons.h>

#ifndef _MILIGHTCLIENT_H
#define _MILIGHTCLIENT_H

#define MILIGHT_PACKET_LENGTH 7
#define MILIGHT_CCT_INTERVALS 10
#define MILIGHT_DEFAULT_RESEND_COUNT 10

enum MiLightStatus { ON = 0, OFF = 1 };

class MiLightRadioStack {
public:
  MiLightRadioStack(RF24& rf, const MiLightRadioConfig& config) 
    : type(config.type)
  {
    nrf = new PL1167_nRF24(rf);
    radio = new MiLightRadio(*nrf, config);
  }
  
  ~MiLightRadioStack() {
    delete radio;
    delete nrf;
  }
  
  inline MiLightRadio* getRadio() {
    return this->radio;
  }
  
  const MiLightRadioType& type;
  
private:
  PL1167_nRF24 *nrf;
  MiLightRadio *radio;
};

class MiLightClient {
  public:
    MiLightClient(uint8_t cePin, uint8_t csnPin)
    : sequenceNum(0),
      rf(RF24(cePin, csnPin)),
      resendCount(MILIGHT_DEFAULT_RESEND_COUNT)
    {
      rgbwRadio = new MiLightRadioStack(rf, MilightRgbwConfig);
      cctRadio = new MiLightRadioStack(rf, MilightCctConfig);
      rgbCctRadio = new MiLightRadioStack(rf, MilightRgbCctConfig);
    }
    
    ~MiLightClient() {
      delete rgbwRadio;
      delete cctRadio;
      delete rgbCctRadio;
    }
    
    void begin() {
      rgbwRadio->getRadio()->begin();
      cctRadio->getRadio()->begin();
      rgbCctRadio->getRadio()->begin();
    }
    
    void setResendCount(const unsigned int resendCount);
    
    bool available(const MiLightRadioType radioType);
    void read(const MiLightRadioType radioType, uint8_t packet[]);
    void write(const MiLightRadioType radioType, uint8_t packet[]);
    
    void writeRgbw(
      const uint16_t deviceId,
      const uint8_t color,
      const uint8_t brightness,
      const uint8_t groupId,
      const uint8_t button
    );
    
    void writeCct(
      const uint16_t deviceId,
      const uint8_t groupId,
      const uint8_t button
    );
    
    // Common methods
    void updateStatus(const MiLightRadioType type,const uint16_t deviceId, const uint8_t groupId, MiLightStatus status);
    void pair(const MiLightRadioType type,const uint16_t deviceId, const uint8_t groupId);
    void unpair(const MiLightRadioType type,const uint16_t deviceId, const uint8_t groupId);
    void allOn(const MiLightRadioType type,const uint16_t deviceId);
    void allOff(const MiLightRadioType type,const uint16_t deviceId);
    void pressButton(const MiLightRadioType type, const uint16_t deviceId, const uint8_t groupId, uint8_t button);
    
    // RGBW methods
    void updateHue(const uint16_t deviceId, const uint8_t groupId, const uint16_t hue);
    void updateBrightness(const uint16_t deviceId, const uint8_t groupId, const uint8_t brightness);
    void updateColorWhite(const uint16_t deviceId, const uint8_t groupId);
    void updateColorRaw(const uint16_t deviceId, const uint8_t groupId, const uint16_t color);

    // CCT methods
    void updateTemperature(const uint16_t deviceId, const uint8_t groupId, const uint8_t colorTemperature);
    void decreaseTemperature(const uint16_t deviceId, const uint8_t groupId);
    void increaseTemperature(const uint16_t deviceId, const uint8_t groupId);
    void updateCctBrightness(const uint16_t deviceId, const uint8_t groupId, const uint8_t brightness);
    void decreaseCctBrightness(const uint16_t deviceId, const uint8_t groupId);
    void increaseCctBrightness(const uint16_t deviceId, const uint8_t groupId);
    
    MiLightRadio* getRadio(const MiLightRadioType type);
    
    static uint8_t getCctStatusButton(uint8_t groupId, MiLightStatus status);
    static MiLightRadioType getRadioType(const String& typeName);
    static const MiLightRadioConfig& getRadioConfig(const String& typeName);
    
    void formatPacket(MiLightRadioConfig& config, uint8_t* packet, char* buffer);
    
  private:
    RF24 rf;
    MiLightRadioStack* rgbwRadio;
    MiLightRadioStack* cctRadio;
    MiLightRadioStack* rgbCctRadio;
    MiLightRadioType currentRadio;
    
    uint8_t sequenceNum;
    uint8_t nextSequenceNum();
    unsigned int resendCount;
};

#endif