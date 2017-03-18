#include <RgbCctPacketFormatter.h>

void RgbCctPacketFormatter::reset() {
  size_t packetPtr = 0;
  
  packet[packetPtr++] = 0x00;
  packet[packetPtr++] = RGB_CCT;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = sequenceNum++;
  packet[packetPtr++] = groupId;
  packet[packetPtr++] = 0;
}

void RgbCctPacketFormatter::updateStatus(MiLightStatus status) {
  packet[RGB_CCT_COMMAND_INDEX] = RGB_CCT_ON;
  packet[RGB_CCT_ARGUMENT_INDEX] = 0xC0 + groupId + (status == OFF ? 5 : 0);
}

void RgbCctPacketFormatter::updateBrightness(uint8_t brightness) {
  packet[RGB_CCT_COMMAND_INDEX] = RGB_CCT_BRIGHTNESS;
  packet[RGB_CCT_ARGUMENT_INDEX] = 0x4F + rescale(brightness, 0x60);
}
