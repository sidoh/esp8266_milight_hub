// Adapated from code from henryk

#include <PL1167_nRF24.h>
#include <NRF24MiLightRadio.h>

#define PACKET_ID(packet, packet_length) ( (packet[1] << 8) | packet[packet_length - 1] )

NRF24MiLightRadio::NRF24MiLightRadio(
  RF24& rf24,
  const MiLightRadioConfig& config,
  const std::vector<RF24Channel>& channels,
  RF24Channel listenChannel
)
  : channels(channels),
    listenChannelIx(static_cast<size_t>(listenChannel)),
    _pl1167(PL1167_nRF24(rf24)),
    _config(config),
    _waiting(false)
{ }

int NRF24MiLightRadio::begin() {
  int retval = _pl1167.open();
  if (retval < 0) {
    return retval;
  }

  retval = configure();
  if (retval < 0) {
    return retval;
  }

  available();

  return 0;
}

int NRF24MiLightRadio::configure() {
  int retval = _pl1167.setSyncword(_config.syncwordBytes, MiLightRadioConfig::SYNCWORD_LENGTH);
  if (retval < 0) {
    return retval;
  }

  // +1 to be able to buffer the length
  retval = _pl1167.setMaxPacketLength(_config.packetLength + 1);
  if (retval < 0) {
    return retval;
  }

  return 0;
}

bool NRF24MiLightRadio::available() {
  if (_waiting) {
#ifdef DEBUG_PRINTF
  printf("_waiting\n");
#endif
    return true;
  }

  if (_pl1167.receive(_config.channels[listenChannelIx]) > 0) {
#ifdef DEBUG_PRINTF
  printf("NRF24MiLightRadio - received packet!\n");
#endif
    size_t packet_length = sizeof(_packet);
    if (_pl1167.readFIFO(_packet, packet_length) < 0) {
      return false;
    }
#ifdef DEBUG_PRINTF
  printf("NRF24MiLightRadio - Checking packet length (expecting %d, is %d)\n", _packet[0] + 1U, packet_length);
#endif
    if (packet_length == 0 || packet_length != _packet[0] + 1U) {
      return false;
    }
    uint32_t packet_id = PACKET_ID(_packet, packet_length);
#ifdef DEBUG_PRINTF
  printf("Packet id: %d\n", packet_id);
#endif
    if (packet_id == _prev_packet_id) {
      _dupes_received++;
    } else {
      _prev_packet_id = packet_id;
      _waiting = true;
    }
  }

  return _waiting;
}

int NRF24MiLightRadio::read(uint8_t frame[], size_t &frame_length)
{
  if (!_waiting) {
    frame_length = 0;
    return -1;
  }

  if (frame_length > sizeof(_packet) - 1) {
    frame_length = sizeof(_packet) - 1;
  }

  if (frame_length > _packet[0]) {
    frame_length = _packet[0];
  }

  memcpy(frame, _packet + 1, frame_length);
  _waiting = false;

  return _packet[0];
}

int NRF24MiLightRadio::write(uint8_t frame[], size_t frame_length) {
  if (frame_length > sizeof(_out_packet) - 1) {
    return -1;
  }

  memcpy(_out_packet + 1, frame, frame_length);
  _out_packet[0] = frame_length;

  int retval = resend();
  if (retval < 0) {
    return retval;
  }
  return frame_length;
}

int NRF24MiLightRadio::resend() {
  for (std::vector<RF24Channel>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
    size_t channelIx = static_cast<uint8_t>(*it);
    uint8_t channel = _config.channels[channelIx];

    _pl1167.writeFIFO(_out_packet, _out_packet[0] + 1);
    _pl1167.transmit(channel);
  }

  return 0;
}

const MiLightRadioConfig& NRF24MiLightRadio::config() {
  return _config;
}
