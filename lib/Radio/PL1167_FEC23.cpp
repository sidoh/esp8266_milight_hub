#include <PL1167_FEC23.h>
#include <bitstreamConvert.h>

/*
    This class is to implement the Hamming 15,10
    for Forward Error Correction FEC23 by pl1167 

    created: 10.08.2023
    author: Michael Balen
    inspired by http://www.finetune.co.jp/~lyuka/technote/ecc/ham1510/
*/

uint32_t PL1167_FEC23::getSyndrome(){
    syndrome = code ^ encodeTable[code >> ECC_LEN];
    status = errorTable[syndrome];
    return syndrome;
};

uint32_t PL1167_FEC23::correct(){
    code ^= burstErrorCorrection ? status >> 16 : status ;
    code &= CODE_MASK;
    return code;
};

uint32_t PL1167_FEC23::encode(uint16_t data){
    code = encodeTable[data & DATA_MASK];
    return code;
};

uint16_t PL1167_FEC23::decode(){
    return (code >> ECC_LEN) & DATA_MASK;
};

size_t PL1167_FEC23::encodeMessage(uint8_t data[], size_t numValues){
    BitstreamConvert converter(8, 10);
    converter.convert(data, numValues);
    uint16_t * out = converter.get();

    for(uint8_t i = 0; i < converter.sizeOut; i++){
        out[i] = encode(out[i]);
    }

    BitstreamConvert converter2(15, 8);
    converter2.convert(out, converter.sizeOut);
    out = converter2.get();

    for(uint8_t i = 0; i < converter2.sizeOut; i++){
        data[i] = out[i];
    }

    return converter2.sizeOut;
};

size_t PL1167_FEC23::decodeMessage(uint8_t data[], size_t numValues){
    BitstreamConvert converter(8, 15);
    converter.convert(data, numValues);
    uint16_t * out = converter.get();

    for(uint8_t i = 0; i < converter.sizeOut; i++){
        code = out[i];
        getSyndrome();
        correct();    
        out[i] = decode();
    }

    BitstreamConvert converter2(10, 8);
    converter2.convert(out, converter.sizeOut);
    out = converter2.get();

    for(uint8_t i = 0; i < converter2.sizeOut; i++){
        data[i] = out[i];
    }

    return converter2.sizeOut;
};

