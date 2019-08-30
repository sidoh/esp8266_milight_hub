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

  // We can set this to two possible values.  It only has an affect on the nRF24 radio.  The
  // LT8900/PL1167 radio will always use the raw syncwords.  For the nRF24, this controls what
  // we set the "address" to, which roughly corresponds to the LT8900 syncword.
  //
  // The PL1167 packet is structured as follows (lengths in bits):
  //  Preamble ( 8) | Syncword (32) | Trailer ( 4) | Packet Len ( 8) | Packet (...)
  //
  // 4 -- Use the raw syncword bits as the address.  This means the Trailer will be included in
  //      the packet data.  Since the Trailer is 4 bits, packet data will not be byte-aligned,
  //      and the data must be bitshifted every time it's received.
  //
  // 5 -- Include the Trailer in the syncword.  Avoids us needing to bitshift packet data. The
  //      downside is that the Trailer is hardcoded and assumed based on received packets.
  //
  // In general, this should be set to 5 unless packets that should be showing up are
  // mysteriously not present.
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
    if (SYNCWORD_LENGTH == 5) {
      syncwordBytes[ --ix ] = reverseBits(
        ((syncword0 << 4) & 0xF0) | (preamble & 0x0F)
      );
      syncwordBytes[ --ix ] = reverseBits((syncword0 >> 4) & 0xFF);
      syncwordBytes[ --ix ] = reverseBits(((syncword0 >> 12) & 0x0F) + ((syncword3 << 4) & 0xF0));
      syncwordBytes[ --ix ] = reverseBits((syncword3 >> 4) & 0xFF);
      syncwordBytes[ --ix ] = reverseBits(
        ((syncword3 >> 12) & 0x0F) | ((trailer << 4) & 0xF0)
      );
    } else {
      syncwordBytes[ --ix ] = reverseBits(syncword0 & 0xff);
      syncwordBytes[ --ix ] = reverseBits( (syncword0 >> 8) & 0xff);
      syncwordBytes[ --ix ] = reverseBits(syncword3 & 0xff);
      syncwordBytes[ --ix ] = reverseBits( (syncword3 >> 8) & 0xff);
    }
  }

  uint8_t channels[3];
  uint8_t syncwordBytes[SYNCWORD_LENGTH];
  uint16_t syncword0, syncword3;

  const size_t packetLength;

  static const size_t NUM_CONFIGS = 5;
  static MiLightRadioConfig ALL_CONFIGS[NUM_CONFIGS];
};

#endif
