/*
 * MiLightRadio.h
 *
 *  Created on: 29 May 2015
 *      Author: henryk
 */

#ifdef ARDUINO
#include "Arduino.h"
#else
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <MiLightRadioConfig.h>
#include <MiLightButtons.h>
#include <MiLightRadioInterface.h>


// Register defines
#define REGISTER_READ       0b10000000  //bin
#define REGISTER_WRITE      0b00000000  //bin
#define REGISTER_MASK       0b01111111  //bin

#define R_CHANNEL           7
#define CHANNEL_RX_BIT      7
#define CHANNEL_TX_BIT      8
#define CHANNEL_MASK        0b01111111  ///bin

#define STATUS_PKT_BIT_MASK	0x40

#define R_STATUS            48
#define STATUS_CRC_BIT      15

#define R_FIFO              50
#define R_FIFO_CONTROL      52

#define R_SYNCWORD1         36
#define R_SYNCWORD2         37
#define R_SYNCWORD3         38
#define R_SYNCWORD4         39

#define DEFAULT_TIME_BETWEEN_RETRANSMISSIONS_uS	350


#ifndef MILIGHTRADIOPL1167_LT8900_H_
#define MILIGHTRADIOPL1167_LT8900_H_

class MiLightRadioPL1167_LT8900 : public MiLightRadioInterface{
  public:
    MiLightRadioPL1167_LT8900(byte byCSPin, byte byResetPin, byte byPktFlag, const MiLightRadioConfig& config);

    virtual int begin();
    virtual bool available();
    virtual int read(uint8_t frame[], size_t &frame_length);
    virtual int dupesReceived();
    virtual int write(uint8_t frame[], size_t frame_length);
    virtual int resend();
    virtual int configure();

  private:


        void vInitRadioModule(MiLightRadioType type);
        void vSetSyncWord(uint16_t syncWord3, uint16_t syncWord2, uint16_t syncWord1, uint16_t syncWord0);
        uint16_t uiReadRegister(uint8_t reg);
        void regWrite16(byte ADDR, byte V1, byte V2, byte WAIT);
        uint8_t uiWriteRegister(uint8_t reg, uint16_t data);

        bool bAvailablePin(void);
        bool bAvailableRegister(void);
        void vStartListening(uint uiChannelToListenTo);
        void vResumeRX(void);
        int iReadRXBuffer(uint8_t *buffer, size_t maxBuffer);
        void vSetChannel(uint8_t channel);
        void vGenericSendPacket(int iMode, int iLength, byte *pbyFrame, byte byChannel );
        bool bCheckRadioConnection(void);
        bool sendPacket(uint8_t *data, size_t packetSize,byte byChannel);

        byte _pin_pktflag;
      	byte _csPin;

    const MiLightRadioConfig& config;
    //uint32_t _prev_packet_id;
    uint8_t _channel = 0;
    uint8_t _packet[10];
    uint8_t _out_packet[10];
    bool _waiting;
    int _dupes_received;
};



#endif /* MILIGHTRADIO_H_ */
