#include <MiLightRadioFactory.h>
#include <SPI.h>

#if defined(ESP32) && defined(USE_ETHERNET)
// HSPI bus remapped off the LAN8720 pins (VSPI 18/19/23).
// Non-static so PL1167_nRF24 can pass it to RF24::begin().
SPIClass ETH_SAFE_SPI(HSPI);
#endif

std::shared_ptr<MiLightRadioFactory> MiLightRadioFactory::fromSettings(const Settings& settings) {
  switch (settings.radioInterfaceType) {
    case nRF24:
      return std::make_shared<NRF24Factory>(
        settings.csnPin,
        settings.cePin,
        settings.rf24PowerLevel,
        settings.rf24Channels,
        settings.rf24ListenChannel
      );

    case LT8900:
      return std::make_shared<LT8900Factory>(settings.csnPin, settings.resetPin, settings.cePin);

    default:
      return NULL;
  }
}

NRF24Factory::NRF24Factory(
  uint8_t csnPin,
  uint8_t cePin,
  RF24PowerLevel rF24PowerLevel,
  const std::vector<RF24Channel>& channels,
  RF24Channel listenChannel
)
: rf24(RF24(cePin, csnPin)),
  channels(channels),
  listenChannel(listenChannel)
{
#if defined(ESP32) && defined(USE_ETHERNET)
  // Must begin the bus before RF24::begin(&ETH_SAFE_SPI) in PL1167_nRF24::open().
  ETH_SAFE_SPI.begin(14, 35, 15, csnPin);
#endif
  rf24.setPALevel(RF24PowerLevelHelpers::rf24ValueFromValue(rF24PowerLevel));
}

std::shared_ptr<MiLightRadio> NRF24Factory::create(const MiLightRadioConfig &config) {
  return std::make_shared<NRF24MiLightRadio>(rf24, config, channels, listenChannel);
}

LT8900Factory::LT8900Factory(uint8_t csPin, uint8_t resetPin, uint8_t pktFlag)
  : _csPin(csPin),
    _resetPin(resetPin),
    _pktFlag(pktFlag)
{ }

std::shared_ptr<MiLightRadio> LT8900Factory::create(const MiLightRadioConfig& config) {
  return std::make_shared<LT8900MiLightRadio>(_csPin, _resetPin, _pktFlag, config);
}
