#include <RadioUtils.h>

#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>

uint8_t reverseBits(uint8_t byte) {
  uint8_t result = byte;
  uint8_t i = 7;

  for (byte >>= 1; byte; byte >>= 1) {
    result <<= 1;
    result |= byte & 1;
    --i;
  }

  return result << i;
}