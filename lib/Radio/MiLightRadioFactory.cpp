#include <MiLightRadioFactory.h>

#if defined(ESP32) && defined(RF24_SPI_SCK) && defined(RF24_SPI_MOSI) && defined(RF24_SPI_MISO)
  #include <SPI.h>
  #define ESPMH_REMAP_RF24_SPI 1
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
#ifdef ESPMH_REMAP_RF24_SPI
  // Bring up the SPI bus on the pins provided by the board variant before
  // RF24::begin() runs. The Arduino ESP32 SPI class ignores the pin arguments
  // if the bus is already initialized, so this is safe to call more than once
  // (e.g. when applySettings() reconstructs the factory).
  SPI.begin(RF24_SPI_SCK, RF24_SPI_MISO, RF24_SPI_MOSI, csnPin);
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
