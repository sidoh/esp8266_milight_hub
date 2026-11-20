#pragma once

#ifdef ESP32
#include <Arduino.h>
#include <SPI.h>
#include <MiLightRadio.h>
#include <MiLightRadioConfig.h>
#include <MiLightRadioFactory.h>
#include <RadioUtils.h>

class HspiNRF24MiLightRadio : public MiLightRadio {
public:
  HspiNRF24MiLightRadio(
    SPIClass& spi,
    uint8_t cePin,
    uint8_t csnPin,
    const MiLightRadioConfig& config
  );

  int begin() override;
  bool available() override;
  int read(uint8_t frame[], size_t &frame_length) override;
  int dupesReceived() { return _dupes_received; }
  int write(uint8_t frame[], size_t frame_length) override { return 0; }
  int resend() override { return 0; }
  int configure() override;
  const MiLightRadioConfig& config() override { return _config; }
  static uint32_t rawFifoHits;
  static uint32_t crcPassCount;
  static uint32_t crcFailCount;
  static bool reverseSyncwordOrder;
  void setCePin(uint8_t cePin) { _cePin = cePin; }
  uint8_t debugReadReg(uint8_t reg) { return readReg(reg); }

private:
  SPIClass& _spi;
  uint8_t _cePin;
  uint8_t _csnPin;
  const MiLightRadioConfig& _config;

  uint8_t _rxLen;
  uint8_t _chanCursor;
  uint32_t _lastHopUs;
  uint32_t _prevPacketId;
  uint32_t _prevPacketMs;
  uint32_t _candHash;
  uint32_t _candMs;
  int _dupes_received;
  bool _waiting;
  uint8_t _packet[16];

  uint8_t readReg(uint8_t reg);
  void writeReg(uint8_t reg, uint8_t val);
  void writeSyncwordReg(uint8_t reg, const uint8_t* buf, size_t len);
  void readPayload(uint8_t* buf, size_t len);
  void flushRx();
  static uint16_t calcCrc(const uint8_t* data, size_t len);
};

class HspiNRF24Factory : public MiLightRadioFactory {
public:
  static SPIClass& sharedHspi() {
    static SPIClass s_hspi(HSPI);
    return s_hspi;
  }

  HspiNRF24Factory(uint8_t cePin, uint8_t csnPin, int8_t sck = 14, int8_t miso = 12, int8_t mosi = 13)
    : _hspi(sharedHspi()), _cePin(cePin), _csnPin(csnPin), _ready(false) {
    pinMode(_cePin, OUTPUT);
    digitalWrite(_cePin, LOW);
    pinMode(_csnPin, OUTPUT);
    digitalWrite(_csnPin, HIGH);
    _hspi.begin(sck, miso, mosi, -1);

    // Probe RF_CH register (0x05) to confirm module presence
    _hspi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_csnPin, LOW);
    delayMicroseconds(5);
    _hspi.transfer(0x20 | 0x05);
    _hspi.transfer(0x2A);
    digitalWrite(_csnPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(_csnPin, LOW);
    delayMicroseconds(5);
    _hspi.transfer(0x05);
    uint8_t rb = _hspi.transfer(0xFF);
    digitalWrite(_csnPin, HIGH);
    _hspi.endTransaction();
    _ready = (rb == 0x2A);
  }

  bool isReady() const { return _ready; }
  SPIClass& spi() { return _hspi; }
  void setCePin(uint8_t cePin) {
    _cePin = cePin;
    pinMode(_cePin, OUTPUT);
    digitalWrite(_cePin, LOW);
  }

  std::shared_ptr<MiLightRadio> create(const MiLightRadioConfig& config) override {
    return std::make_shared<HspiNRF24MiLightRadio>(_hspi, _cePin, _csnPin, config);
  }

private:
  SPIClass& _hspi;
  uint8_t _cePin;
  uint8_t _csnPin;
  bool _ready;
};
#endif
