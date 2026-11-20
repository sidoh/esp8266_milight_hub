#ifdef ESP32
#include <HspiNRF24MiLightRadio.h>

#define PL1167_CRC_POLY 0x8408

static uint32_t hashPayload(const uint8_t* pkt, size_t len) {
  uint32_t h = 2166136261u;
  for (size_t i = 0; i < len; i++) {
    h ^= pkt[i];
    h *= 16777619u;
  }
  return h;
}

uint32_t HspiNRF24MiLightRadio::rawFifoHits = 0;
uint32_t HspiNRF24MiLightRadio::crcPassCount = 0;
uint32_t HspiNRF24MiLightRadio::crcFailCount = 0;
bool HspiNRF24MiLightRadio::reverseSyncwordOrder = false;

HspiNRF24MiLightRadio::HspiNRF24MiLightRadio(
  SPIClass& spi, uint8_t cePin, uint8_t csnPin, const MiLightRadioConfig& config
) : _spi(spi), _cePin(cePin), _csnPin(csnPin), _config(config)
  , _rxLen((uint8_t)(config.packetLength + 1 + 2))
  , _chanCursor(1) // Park on MID channel (ch 39 -> 2441 MHz, clean between Wi-Fi ch 1 & ch 11!)
  , _lastHopUs(0)
  , _prevPacketId(0), _prevPacketMs(0)
  , _candHash(0), _candMs(0)
  , _dupes_received(0), _waiting(false)
{
  if (MiLightRadioConfig::SYNCWORD_LENGTH < 5) _rxLen++;
}

uint8_t HspiNRF24MiLightRadio::readReg(uint8_t reg) {
  _spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_csnPin, LOW);
  _spi.transfer(reg & 0x1F);
  uint8_t val = _spi.transfer(0xFF);
  digitalWrite(_csnPin, HIGH);
  _spi.endTransaction();
  return val;
}

void HspiNRF24MiLightRadio::writeReg(uint8_t reg, uint8_t val) {
  _spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_csnPin, LOW);
  _spi.transfer(0x20 | (reg & 0x1F));
  _spi.transfer(val);
  digitalWrite(_csnPin, HIGH);
  _spi.endTransaction();
}

void HspiNRF24MiLightRadio::writeSyncwordReg(uint8_t reg, const uint8_t* buf, size_t len) {
  _spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_csnPin, LOW);
  _spi.transfer(0x20 | (reg & 0x1F));
  if (reverseSyncwordOrder) {
    for (size_t i = len; i > 0; i--) _spi.transfer(buf[i - 1]);
  } else {
    for (size_t i = 0; i < len; i++) _spi.transfer(buf[i]);
  }
  digitalWrite(_csnPin, HIGH);
  _spi.endTransaction();
}

void HspiNRF24MiLightRadio::readPayload(uint8_t* buf, size_t len) {
  _spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_csnPin, LOW);
  _spi.transfer(0x61);
  for (size_t i = 0; i < len; i++) buf[i] = _spi.transfer(0xFF);
  digitalWrite(_csnPin, HIGH);
  _spi.endTransaction();
}

void HspiNRF24MiLightRadio::flushRx() {
  _spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_csnPin, LOW);
  _spi.transfer(0xE2);
  digitalWrite(_csnPin, HIGH);
  _spi.endTransaction();
}

int HspiNRF24MiLightRadio::begin() {
  _waiting = false;
  digitalWrite(_cePin, LOW);
  writeReg(0x00, 0x03); // PWR_UP=1, PRIM_RX=1, CRC disabled
  delay(5);
  writeReg(0x01, 0x00); // EN_AA = 0
  writeReg(0x02, 0x02); // Pipe 1 only
  writeReg(0x03, (MiLightRadioConfig::SYNCWORD_LENGTH == 5) ? 0x03 : 0x02);
  writeReg(0x04, 0x00); // SETUP_RETR = 0
  writeReg(0x06, 0x07); // 1Mbps, 0dBm + LNA_HCURR
  writeReg(0x1C, 0x00); // DYNPD = 0
  writeReg(0x1D, 0x00); // FEATURE = 0
  return configure();
}

int HspiNRF24MiLightRadio::configure() {
  _waiting = false;
  digitalWrite(_cePin, LOW);
  writeReg(0x00, 0x03);
  writeReg(0x02, 0x02);
  writeSyncwordReg(0x0B, _config.syncwordBytes, MiLightRadioConfig::SYNCWORD_LENGTH);
  writeReg(0x12, _rxLen);
  // Fixed listen channel matching upstream NRF24MiLightRadio (ch[1] = MID, zero hop blackouts!)
  writeReg(0x05, 2 + _config.channels[1]);
  flushRx();
  writeReg(0x07, 0x70);
  digitalWrite(_cePin, HIGH);
  delayMicroseconds(130);
  return 0;
}

bool HspiNRF24MiLightRadio::available() {
  if (_waiting) {
    _dupes_received = 255;
    return true;
  }

  for (int slot = 0; slot < 3; slot++) {
    uint8_t status = readReg(0x07);
    uint8_t fifoStatus = readReg(0x17);
    if (status == 0xFF || fifoStatus == 0xFF || fifoStatus == 0x00) {
      return false;
    }
    bool rxEmpty = (fifoStatus & 0x01) != 0;
    if (rxEmpty && !(status & 0x40)) {
      break;
    }

    rawFifoHits++;
    uint8_t tmp[32];
    size_t readLen = (_rxLen <= sizeof(tmp)) ? _rxLen : sizeof(tmp);
    readPayload(tmp, readLen);
    writeReg(0x07, 0x40);

    for (size_t i = 0; i < readLen; i++) {
      tmp[i] = reverseBits(tmp[i]);
    }

    if (readLen >= 3) {
      uint16_t crc = calcCrc(tmp, readLen - 2);
      uint16_t recvCrc = ((uint16_t)tmp[readLen - 1] << 8) | tmp[readLen - 2];
      size_t payloadLen = readLen - 2;

      if (crc == recvCrc && payloadLen > 0 && payloadLen == (size_t)(tmp[0] + 1U) && payloadLen <= sizeof(_packet)) {
        crcPassCount++;
        uint32_t pktHash = hashPayload(tmp, payloadLen);
        uint32_t nowMs = millis();

        if (pktHash == _prevPacketId && (nowMs - _prevPacketMs < 140)) {
          _dupes_received++;
          continue;
        }

        // Require 2 matching frames on fixed MID channel (every B8 burst sends 10+ frames on ch 39!)
        if (pktHash == _candHash && (nowMs - _candMs < 140)) {
          _prevPacketId = pktHash;
          _prevPacketMs = nowMs;
          _candHash = 0;
          memcpy(_packet, tmp, payloadLen);
          _dupes_received = 255;
          _waiting = true;
          return true;
        } else {
          _candHash = pktHash;
          _candMs = nowMs;
        }
      } else {
        crcFailCount++;
        // Match upstream PL1167_nRF24::internal_receive() — re-arm Pipe 1 bit-sync after a bad CRC!
        digitalWrite(_cePin, LOW);
        flushRx();
        writeReg(0x07, 0x70);
        digitalWrite(_cePin, HIGH);
        break;
      }
    }
  }

  return false;
}

int HspiNRF24MiLightRadio::read(uint8_t frame[], size_t &frame_length) {
  if (!_waiting) {
    frame_length = 0;
    return -1;
  }
  size_t pktLen = _packet[0];
  if (pktLen > MILIGHT_MAX_PACKET_LENGTH) pktLen = MILIGHT_MAX_PACKET_LENGTH;
  // Always write full pktLen (up to 9 bytes) and update frame_length to pktLen
  frame_length = pktLen;
  memcpy(frame, _packet + 1, pktLen);
  _waiting = false;
  _dupes_received = 0;
  return (int)pktLen;
}

uint16_t HspiNRF24MiLightRadio::calcCrc(const uint8_t* data, size_t data_length) {
  uint16_t state = 0;
  for (size_t i = 0; i < data_length; i++) {
    uint8_t b = data[i];
    for (int j = 0; j < 8; j++) {
      state = ((b ^ state) & 0x01) ? ((state >> 1) ^ PL1167_CRC_POLY) : (state >> 1);
      b >>= 1;
    }
  }
  return state;
}
#endif
