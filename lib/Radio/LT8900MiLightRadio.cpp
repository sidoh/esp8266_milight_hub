/*
 * MiLightRadioPL1167_LT89000.cpp
 *
 *  Created on: 31 March 2017
 *      Author: WoodsterDK
 *
 *  Very inspired by:
 *  https://github.com/pmoscetta/authometion-milight/tree/master/Authometion-MiLight
 *  https://bitbucket.org/robvanderveer/lt8900lib
 */

#include "LT8900MiLightRadio.h"
#include <SPI.h>

/**************************************************************************/
// Constructor
/**************************************************************************/
LT8900MiLightRadio::LT8900MiLightRadio(byte byCSPin, byte byResetPin, byte byPktFlag, const MiLightRadioConfig& config)
  : _config(config),
    _channel(0),
    _currentPacketLen(0),
    _currentPacketPos(0)
{
  _csPin = byCSPin;
	_pin_pktflag = byPktFlag;

  pinMode(_pin_pktflag, INPUT);

	if (byResetPin > 0)					// If zero then bypass hardware reset
	{
		pinMode(byResetPin, OUTPUT);
		digitalWrite(byResetPin, LOW);
		delay(200);
		digitalWrite(byResetPin, HIGH);
		delay(200);
	}

  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH);

  SPI.begin();

  SPI.setDataMode(SPI_MODE1);
  // The following speed settings depends upon the wiring and PCB
  //SPI.setFrequency(8000000);
  SPI.setFrequency(4000000);
  SPI.setBitOrder(MSBFIRST);

  //Initialize transceiver with correct settings
  vInitRadioModule();
  delay(50);

  // Check if HW is connected
  _bConnected = bCheckRadioConnection();

  //Reset SPI MODE to default
  SPI.setDataMode(SPI_MODE0);
  _waiting = false;
}




/**************************************************************************/
// Checks the connection to the radio module by verifying a register setting
/**************************************************************************/
bool LT8900MiLightRadio::bCheckRadioConnection(void)
{
	bool bRetValue = false;
	uint16_t value_0 = uiReadRegister(0);
	uint16_t value_1 = uiReadRegister(1);

	if ((value_0 == 0x6fe0) && (value_1 == 0x5681))
	{
    #ifdef DEBUG_PRINTF
		  Serial.println(F("Radio module running correctly..."));
    #endif
		bRetValue = true;
	}
	else
	{
    #ifdef DEBUG_PRINTF
		  Serial.println(F("Failed initializing the radio module..."));
    #endif
	}

	return bRetValue;
}

/**************************************************************************/
// Initialize radio module
/**************************************************************************/
void LT8900MiLightRadio::vInitRadioModule() {
	bool bWriteDefaultDefault = true;  // Is it okay to use the default power up values, without setting them

	regWrite16(0x00, 0x6F, 0xE0, 7);  // Recommended value by PMmicro
	regWrite16(0x02, 0x66, 0x17, 7);  // Recommended value by PMmicro
	regWrite16(0x04, 0x9C, 0xC9, 7);  // Recommended value by PMmicro

	regWrite16(0x05, 0x66, 0x37, 7);  // Recommended value by PMmicro
	regWrite16(0x07, 0x00, 0x4C, 7);  // PL1167's TX/RX Enable and Channel Register, Default channel 76
	regWrite16(0x08, 0x6C, 0x90, 7);  // Recommended value by PMmicro
	regWrite16(0x09, 0x48, 0x00, 7);  // PA Control register

	regWrite16(0x0B, 0x00, 0x08, 7);  // Recommended value by PMmicro
	regWrite16(0x0D, 0x48, 0xBD, 7);  // Recommended value by PMmicro
	regWrite16(0x16, 0x00, 0xFF, 7);  // Recommended value by PMmicro
	regWrite16(0x18, 0x00, 0x67, 7);  // Recommended value by PMmicro

	regWrite16(0x1A, 0x19, 0xE0, 7);  // Recommended value by PMmicro
	regWrite16(0x1B, 0x13, 0x00, 7);  // Recommended value by PMmicro

	regWrite16(0x20, 0x48, 0x00, 7);  // Recommended value by PMmicro
	regWrite16(0x21, 0x3F, 0xC7, 7);  // Recommended value by PMmicro
	regWrite16(0x22, 0x20, 0x00, 7);  // Recommended value by PMmicro
	regWrite16(0x23, 0x03, 0x00, 7);  // Recommended value by PMmicro

	regWrite16(0x24, 0x72, 0x36, 7);  // Sync R0
	regWrite16(0x27, 0x18, 0x09, 7);  // Sync R3
	regWrite16(0x28, 0x44, 0x02, 7);  // Recommended value by PMmicro
	regWrite16(0x29, 0xB0, 0x00, 7);  // Recommended value by PMmicro
	regWrite16(0x2A, 0xFD, 0xB0, 7);  // Recommended value by PMmicro

	if (bWriteDefaultDefault == true) {
		regWrite16(0x01, 0x56, 0x81, 7);  // Recommended value by PMmicro
		regWrite16(0x0A, 0x7F, 0xFD, 7);  // Recommended value by PMmicro
		regWrite16(0x0C, 0x00, 0x00, 7);  // Recommended value by PMmicro
		regWrite16(0x17, 0x80, 0x05, 7);  // Recommended value by PMmicro
		regWrite16(0x19, 0x16, 0x59, 7);  // Recommended value by PMmicro
		regWrite16(0x1C, 0x18, 0x00, 7);  // Recommended value by PMmicro

		regWrite16(0x25, 0x00, 0x00, 7);  // Recommended value by PMmicro
		regWrite16(0x26, 0x00, 0x00, 7);  // Recommended value by PMmicro
		regWrite16(0x2B, 0x00, 0x0F, 7);  // Recommended value by PMmicro
	}
}

/**************************************************************************/
// Set sync word
/**************************************************************************/
void LT8900MiLightRadio::vSetSyncWord(uint16_t syncWord3, uint16_t syncWord2, uint16_t syncWord1, uint16_t syncWord0)
{
	uiWriteRegister(R_SYNCWORD1, syncWord0);
	uiWriteRegister(R_SYNCWORD2, syncWord1);
	uiWriteRegister(R_SYNCWORD3, syncWord1);
	uiWriteRegister(R_SYNCWORD4, syncWord3);
}

/**************************************************************************/
// Low level register write with delay
/**************************************************************************/
void LT8900MiLightRadio::regWrite16(byte ADDR, byte V1, byte V2, byte WAIT)
{
	digitalWrite(_csPin, LOW);
	SPI.transfer(ADDR);
	SPI.transfer(V1);
	SPI.transfer(V2);
	digitalWrite(_csPin, HIGH);
	delayMicroseconds(WAIT);
}


/**************************************************************************/
// Low level register read
/**************************************************************************/
uint16_t LT8900MiLightRadio::uiReadRegister(uint8_t reg)
{
	SPI.setDataMode(SPI_MODE1);
	digitalWrite(_csPin, LOW);
	SPI.transfer(REGISTER_READ | (REGISTER_MASK & reg));
	uint8_t high = SPI.transfer(0x00);
	uint8_t low = SPI.transfer(0x00);

	digitalWrite(_csPin, HIGH);

	SPI.setDataMode(SPI_MODE0);
	return (high << 8 | low);
}


/**************************************************************************/
// Low level 16bit register write
/**************************************************************************/
uint8_t LT8900MiLightRadio::uiWriteRegister(uint8_t reg, uint16_t data)
{
	uint8_t high = data >> 8;
	uint8_t low = data & 0xFF;

	digitalWrite(_csPin, LOW);

	uint8_t result = SPI.transfer(REGISTER_WRITE | (REGISTER_MASK & reg));
	SPI.transfer(high);
	SPI.transfer(low);

	digitalWrite(_csPin, HIGH);

	return result;
}

/**************************************************************************/
// Start listening on specified channel and syncword
/**************************************************************************/
void LT8900MiLightRadio::vStartListening(uint uiChannelToListenTo)
{
  _dupes_received = 0;
  vSetSyncWord(_config.syncword3, 0,0,_config.syncword0);
	//vSetChannel(uiChannelToListenTo);

  _channel = uiChannelToListenTo;

  vResumeRX();
	delay(5);
}

/**************************************************************************/
// Resume listening - without changing the channel and syncword
/**************************************************************************/
void LT8900MiLightRadio::vResumeRX(void)
{
  _dupes_received = 0;
	uiWriteRegister(R_CHANNEL, _channel & CHANNEL_MASK);   //turn off rx/tx
	delay(3);
	uiWriteRegister(R_FIFO_CONTROL, 0x0080);  //flush rx
	uiWriteRegister(R_CHANNEL, (_channel & CHANNEL_MASK) | _BV(CHANNEL_RX_BIT));   //enable RX
}

/**************************************************************************/
// Check if data is available using the hardware pin PKT_FLAG
/**************************************************************************/
bool LT8900MiLightRadio::bAvailablePin() {
  return digitalRead(_pin_pktflag) > 0;
}

/**************************************************************************/
// Check if data is available using the PKT_FLAG state in the status register
/**************************************************************************/
bool LT8900MiLightRadio::bAvailableRegister() {
	//read the PKT_FLAG state; this can also be done with a hard wire.
	uint16_t value = uiReadRegister(R_STATUS);

  if (bitRead(value, STATUS_CRC_BIT) != 0) {
#ifdef DEBUG_PRINTF
    Serial.println(F("LT8900: CRC failed"));
#endif
    vResumeRX();
    return false;
  }

  return (value & STATUS_PKT_BIT_MASK) > 0;
}

/**************************************************************************/
// Read the RX buffer
/**************************************************************************/
int LT8900MiLightRadio::iReadRXBuffer(uint8_t *buffer, size_t maxBuffer) {
  size_t bufferIx = 0;
  uint16_t data;

  if (_currentPacketLen == 0) {
    if (! available()) {
      return -1;
    }

    data = uiReadRegister(R_FIFO);

    _currentPacketLen = (data >> 8);
    _currentPacketPos = 1;

    buffer[bufferIx++] = (data & 0xFF);
  }

  while (_currentPacketPos < _currentPacketLen && (bufferIx+1) < maxBuffer) {
    data = uiReadRegister(R_FIFO);
    buffer[bufferIx++] = data >> 8;
    buffer[bufferIx++] = data & 0xFF;

    _currentPacketPos += 2;
  }

#ifdef DEBUG_PRINTF
  printf_P(
    PSTR("Read %d/%d bytes in RX, read %d bytes into buffer\n"),
    _currentPacketPos,
    _currentPacketLen,
    bufferIx
  );
#endif

  if (_currentPacketPos >= _currentPacketLen) {
    _currentPacketPos = 0;
    _currentPacketLen = 0;
  }

  return bufferIx;
}


/**************************************************************************/
// Set the active channel for the radio module
/**************************************************************************/
void LT8900MiLightRadio::vSetChannel(uint8_t channel)
{
	_channel = channel;
	uiWriteRegister(R_CHANNEL, (_channel & CHANNEL_MASK));
}

/**************************************************************************/
// Startup
/**************************************************************************/
int LT8900MiLightRadio::begin()
{
  vSetChannel(_config.channels[0]);
  configure();
  available();
  return 0;
}

/**************************************************************************/
// Configure the module according to type, and start listening
/**************************************************************************/
int LT8900MiLightRadio::configure()
{
  vInitRadioModule();
  vSetSyncWord(_config.syncword3, 0,0,_config.syncword0);
  vStartListening(_config.channels[0]);
  return 0;
}

/**************************************************************************/
// Check if data is available
/**************************************************************************/
bool LT8900MiLightRadio::available()
{
  if (_currentPacketPos < _currentPacketLen) {
    return true;
  }

  return bAvailablePin() && bAvailableRegister();
}

/**************************************************************************/
// Read received data from buffer to upper layer
/**************************************************************************/
int LT8900MiLightRadio::read(uint8_t frame[], size_t &frame_length)
{
  if (!available()) {
    frame_length = 0;
    return -1;
  }

  #ifdef DEBUG_PRINTF
  Serial.println(F("LT8900: Radio was available, reading packet..."));
  #endif

  uint8_t buf[MILIGHT_MAX_PACKET_LENGTH];
  int packetSize = iReadRXBuffer(buf, MILIGHT_MAX_PACKET_LENGTH);

  if (packetSize > 0) {
    frame_length = packetSize;
    memcpy(frame, buf, packetSize);
  }

  vResumeRX();

  return packetSize;
}

/**************************************************************************/
// Write data
/**************************************************************************/
int LT8900MiLightRadio::write(uint8_t frame[], size_t frame_length)
{
  if (frame_length > sizeof(_out_packet) - 1) {
    return -1;
  }

  memcpy(_out_packet + 1, frame, frame_length);
  _out_packet[0] = frame_length;

  SPI.setDataMode(SPI_MODE1);

  int retval = resend();
  yield();

  SPI.setDataMode(SPI_MODE0);

  if (retval < 0) {
    return retval;
  }
  return frame_length;
}

/**************************************************************************/
// Handle the transmission to regarding to freq diversity and repeats
/**************************************************************************/
int LT8900MiLightRadio::resend()
{
  byte Length =  _out_packet[0];

  for (size_t i = 0; i < MiLightRadioConfig::NUM_CHANNELS; i++)
  {
    sendPacket(_out_packet, Length, _config.channels[i]);
    delayMicroseconds(DEFAULT_TIME_BETWEEN_RETRANSMISSIONS_uS);
  }

  return 0;
}

/**************************************************************************/
// The actual transmit happens here
/**************************************************************************/
bool LT8900MiLightRadio::sendPacket(uint8_t *data, size_t packetSize, byte byChannel)
{
  if(_bConnected) // Must be connected to module otherwise it might lookup waiting for _pin_pktflag
  {
    if (packetSize < 1 || packetSize > 255)
    {
      return false;
    }

    uiWriteRegister(R_CHANNEL, 0x0000);
    uiWriteRegister(R_FIFO_CONTROL, 0x8080);  //flush tx and RX

    digitalWrite(_csPin, LOW);        // Enable PL1167 SPI transmission
    SPI.transfer(R_FIFO);             // Start writing PL1167's FIFO Data register
    SPI.transfer(packetSize);         // Length of data buffer: x bytes

    for (byte iCounter = 0; iCounter < packetSize; iCounter++)
    {
      SPI.transfer((data[1+iCounter]));
    }
    digitalWrite(_csPin, HIGH);  // Disable PL1167 SPI transmission
    delayMicroseconds(10);

    uiWriteRegister(R_CHANNEL,  (byChannel & CHANNEL_MASK) | _BV(CHANNEL_TX_BIT));   //enable RX

    //Wait until the packet is sent.
    while (digitalRead(_pin_pktflag) == 0)
    {
        //do nothing.
    }

    return true;
  }

  return false;
}

const MiLightRadioConfig& LT8900MiLightRadio::config() {
  return _config;
}
