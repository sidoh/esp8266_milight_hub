#include <Arduino.h>
#include <MiLightRemoteType.h>
#include <Size.h>
#include <RadioUtils.h>

#ifndef _MILIGHT_RADIO_CONFIG
#define _MILIGHT_RADIO_CONFIG

#define MILIGHT_MAX_PACKET_LENGTH 9

class MiLightRadioConfig {
public:
  static const size_t NUM_CHANNELS = 3;
  static const uint8_t SYNCWORD_LENGTH = 5;

  MiLightRadioConfig(
    const uint16_t syncword0,
    const uint16_t syncword3,
    const size_t packetLength,
    const uint8_t channel0,
    const uint8_t channel1,
    const uint8_t channel2,
    const uint8_t preamble,
    const uint8_t trailer
  ) : syncword0(syncword0)
    , syncword3(syncword3)
    , packetLength(packetLength)
  {
    channels[0] = channel0;
    channels[1] = channel1;
    channels[2] = channel2;

    size_t ix = SYNCWORD_LENGTH;

    // precompute the syncword for the nRF24.  we include the fixed preamble and trailer in the
    // syncword to avoid needing to bitshift packets.  trailer is 4 bits, so the actual syncword
    // is no longer byte-aligned.
    syncwordBytes[ --ix ] = reverseBits(
      ((syncword0 << 4) & 0xF0) | (preamble & 0x0F)
    );
    syncwordBytes[ --ix ] = reverseBits((syncword0 >> 4) & 0xFF);
    syncwordBytes[ --ix ] = reverseBits(((syncword0 >> 12) & 0x0F) + ((syncword3 << 4) & 0xF0));
    syncwordBytes[ --ix ] = reverseBits((syncword3 >> 4) & 0xFF);
    syncwordBytes[ --ix ] = reverseBits(
      ((syncword3 >> 12) & 0x0F) | ((trailer << 4) & 0xF0)
    );

    // Uncomment this and change syncword length to 4 to disable preamble/trailer being part of
    // the address.
    // syncwordBytes[ --ix ] = reverseBits(syncword0 & 0xff);
    // syncwordBytes[ --ix ] = reverseBits( (syncword0 >> 8) & 0xff);
    // syncwordBytes[ --ix ] = reverseBits(syncword3 & 0xff);
    // syncwordBytes[ --ix ] = reverseBits( (syncword3 >> 8) & 0xff);
  }

  uint8_t channels[3];
  uint8_t syncwordBytes[SYNCWORD_LENGTH];
  uint16_t syncword0, syncword3;

  const size_t packetLength;

  static const size_t NUM_CONFIGS = 5;
  static MiLightRadioConfig ALL_CONFIGS[NUM_CONFIGS];
};

#endif
