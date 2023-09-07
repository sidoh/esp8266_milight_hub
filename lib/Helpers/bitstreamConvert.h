#include <Arduino.h>

#ifndef _BITSTREAMCONVERT_H
#define _BITSTREAMCONVERT_H

class BitstreamConvert {
    public:
        uint8_t sizeOut = 0;
        
        BitstreamConvert(uint8_t sourceSize, uint8_t destSize){
            src_bitlength = sourceSize;
            dst_bitlength = destSize;
            mask = (1 << dst_bitlength) -1;
            mask <<= 32 -dst_bitlength;
        }

        bool convert(uint16_t input[], uint8_t size){
        #ifdef DEBUG_PRINTF
            printf("converting: %d values %d-bit values\n", size, src_bitlength);
        #endif
            for(uint8_t i = 0; i < size; i++){
                push(input[i]);
            }
            return true;
        }

         bool convert(uint8_t input[], uint8_t size){
        #ifdef DEBUG_PRINTF
            printf("converting: %d %d-bit values\n", size, src_bitlength);
        #endif
            for(uint8_t i = 0; i < size; i++){
                push(input[i]);
            }
            return true;
        }


        uint16_t * get(){
        #ifdef DEBUG_PRINTF
            printf("get %d %d-bit values\n", sizeOut, dst_bitlength);
        #endif
            return output;
        };

    private:
        uint16_t output[32];
        uint8_t bufferPos = 0;
        uint32_t buffer = 0x00000000;
        
        uint8_t src_bitlength;
        uint8_t dst_bitlength;
        uint32_t mask;
        uint32_t outmask;
        
        void push(uint16_t data){
            bufferPos += src_bitlength;
    
            buffer |= data << (32 -bufferPos);

            while(bufferPos >= dst_bitlength){
                uint32_t outbyte = (buffer & mask) >> (32 -dst_bitlength);
                output[sizeOut++] = outbyte;
                buffer <<= dst_bitlength;
                bufferPos -= dst_bitlength;
            }
        }
};


#endif