#include <V6MiLightUdpServer.h>
#include <ESP8266WiFi.h>
  
uint8_t V6MiLightUdpServer::START_SESSION_COMMAND[] = {
  0x20, 0x00, 0x00, 0x00, 0x16, 0x02, 0x62, 0x3A, 0xD5, 0xED, 0xA3, 0x01, 0xAE, 
  0x08, 0x2D, 0x46, 0x61, 0x41, 0xA7, 0xF6, 0xDC, 0xAF, 0xD3, 0xE6, 0x00, 0x00,
  0x1E
};

uint8_t V6MiLightUdpServer::START_SESSION_RESPONSE[] = {
   0x28, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // should be replaced with hw addr
   0x69, 0xF0, 0x3C, 0x23, 0x00, 0x01,
   0xFF, 0xFF, // should be replaced with a session ID
   0x00
};

uint8_t V6MiLightUdpServer::COMMAND_HEADER[] = {
  0x80, 0x00, 0x00, 0x00
};

template<typename T, size_t sz>
size_t size(T(&)[sz]) {
    return sz;
}

template <typename T>
T V6MiLightUdpServer::readInt(uint8_t* packet) {
  size_t numBytes = sizeof(T);
  T value = 0;
  
  for (size_t i = 0; i < numBytes; i++) {
    value |= packet[i] << (8 * (numBytes - i - 1));
  }
  
  return value;
}

template <typename T>
uint8_t* V6MiLightUdpServer::writeInt(const T& value, uint8_t* packet) {
  size_t numBytes = sizeof(T);
  
  for (size_t i = 0; i < numBytes; i++) {
    packet[i] = (value >> (8 * (numBytes - i - 1))) & 0xFF;
  }
  
  return packet + numBytes;
}
    
uint16_t V6MiLightUdpServer::beginSession() {
  const uint16_t id = sessionId++;
  
  V6Session session(socket.remoteIP(), socket.remotePort(), id);
  sessions.push_back(session);
  return id;
}

void V6MiLightUdpServer::handleStartSession() {
  size_t len = size(START_SESSION_RESPONSE);
  uint8_t response[len];
  uint16_t sessionId = beginSession();
  
  memcpy(response, START_SESSION_RESPONSE, len);
  WiFi.macAddress(response + 7);
  response[19] = sessionId >> 8;
  response[20] = sessionId & 0xFF;
  
  sendResponse(sessionId, response, len);
}
  
void V6MiLightUdpServer::sendResponse(uint16_t sessionId, uint8_t* responseBuffer, size_t responseSize) {
  V6Session* session = NULL;
  
  for (size_t i = 0; i < sessions.size(); i++) {
    if (sessions[i].sessionId == sessionId) {
      session = &sessions[i];
    }
  }
  
  if (session == NULL) {
    Serial.print("Tried to send response to untracked session id: ");
    Serial.println(sessionId);
  }
  
  Serial.println("Sending a response!");
  Serial.print("IP: ");
  Serial.println(session->ipAddr.toString());
  Serial.print("Port: ");
  Serial.println(session->port);
  
  socket.beginPacket(session->ipAddr, session->port);
  socket.write(responseBuffer, responseSize);
  socket.endPacket();
}

bool V6MiLightUdpServer::handleV1BulbCommand(uint8_t group, uint32_t _cmd, uint32_t _arg) {
}

bool V6MiLightUdpServer::handleV2BulbCommand(uint8_t group, uint32_t _cmd, uint32_t _arg) {
  const uint8_t cmd = _cmd & 0xFF;
  const uint8_t arg = _arg >> 24;
  
  client->prepare(MilightRgbCctConfig, deviceId, group);
  
  switch (cmd) {
    case V2_STATUS:
      if (arg == 0x01) {
        client->updateStatus(ON);
      } else if (arg == 0x02) {
        client->updateStatus(OFF);
      } else if (arg == 0x05) {
        client->updateBrightness(0);
      }
      break;
      
    case V2_COLOR:
      client->updateColorRaw(arg);
      break;
      
    case V2_KELVIN:
      client->updateTemperature(arg);
      break;
      
    case V2_BRIGHTNESS:
      client->updateBrightness(arg);
      break;
      
    case V2_SATURATION:
      client->updateSaturation(arg);
      break;
      
    default:
      return false;
  }
  
  return true;
}
  
void V6MiLightUdpServer::handleCommand(
  uint16_t sessionId,
  uint8_t sequenceNum,
  uint8_t* cmd,
  uint8_t group,
  uint8_t checksum
) {
  
  uint8_t cmdType = readInt<uint8_t>(cmd);
  uint32_t cmdHeader = readInt<uint32_t>(cmd+1);
  uint32_t cmdArg = readInt<uint32_t>(cmd+5);
  
#ifdef MILIGHT_UDP_DEBUG
  printf("Command type: %02X, command: %08X, arg: %08X\n", cmdType, cmdHeader, cmdArg);
#endif
  
  bool handled = false;
  
  if ((cmdHeader & 0x0800) == 0x0800) {
    printf("Yup.\n");
    handled = handleV2BulbCommand(group, cmdHeader, cmdArg);
  } else if ((cmdHeader & 0x0700) == 0x0700) {
    handled = handleV1BulbCommand(group, cmdHeader, cmdArg);
  }
  
  if (handled) {
    uint8_t* responsePacket = this->responseBuffer;
    uint8_t* packetStart = responseBuffer;
    
    responsePacket = writeInt<uint32_t>(0x88000000, responsePacket);
    responsePacket = writeInt<uint16_t>(0x0300, responsePacket);
    responsePacket = writeInt<uint8_t>(sequenceNum, responsePacket);
    responsePacket = writeInt<uint8_t>(0, responsePacket);
    
    sendResponse(sessionId, packetStart, responsePacket - packetStart);
    
    return;
  }
  
#ifdef MILIGHT_UDP_DEBUG
  printf("V6MiLightUdpServer - Unhandled command: ");
  for (size_t i = 0; i < V6_COMMAND_LEN; i++) {
    printf("%02X ", cmd[i]);
  }
  printf("\n");
#endif
}

void V6MiLightUdpServer::handlePacket(uint8_t* packet, size_t packetSize) {
  printf("Packet size: %d\n", packetSize);
  
  if (packetSize == size(START_SESSION_COMMAND) && memcmp(START_SESSION_COMMAND, packet, packetSize) == 0) {
    handleStartSession();
  } else if (packetSize == 22 && memcmp(COMMAND_HEADER, packet, size(COMMAND_HEADER)) == 0) {
    uint16_t sessionId = (packet[5] << 8) | packet[6];
    uint8_t sequenceNum = packet[8];
    uint8_t* cmd = packet+10;
    uint8_t group = packet[19];
    uint8_t checksum = packet[21];
    
#ifdef MILIGHT_UDP_DEBUG
    printf("session: %04X, sequence: %d, group: %d, checksum: %d\n", sessionId, sequenceNum, group, checksum);
#endif
    
    handleCommand(sessionId, sequenceNum, cmd, group, checksum);
  } else {
    Serial.println("Unhandled V6 packet");
  }
}