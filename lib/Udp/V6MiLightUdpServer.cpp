#include <V6MiLightUdpServer.h>
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <Size.h>
#include <V6CommandHandler.h>

#define MATCHES_PACKET(packet1) ( \
  matchesPacket(packet1, size(packet1), packet, packetSize) \
)

V6CommandDemuxer V6MiLightUdpServer::COMMAND_DEMUXER = V6CommandDemuxer(
  V6CommandHandler::ALL_HANDLERS,
  V6CommandHandler::NUM_HANDLERS
);

uint8_t V6MiLightUdpServer::START_SESSION_COMMAND[] = {
  0x20, 0x00, 0x00, 0x00, 0x16, 0x02, 0x62, 0x3A, 0xD5, 0xED, 0xA3, 0x01, 0xAE,
  0x08, 0x2D, 0x46, 0x61, 0x41, 0xA7, 0xF6, 0xDC, 0xAF
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

uint8_t V6MiLightUdpServer::HEARTBEAT_HEADER[] = {
  0xD0, 0x00, 0x00, 0x00, 0x02
};

uint8_t V6MiLightUdpServer::HEARTBEAT_HEADER2[] = {
  0x30, 0x00, 0x00, 0x00, 0x03
};

uint8_t V6MiLightUdpServer::COMMAND_RESPONSE[] = {
  0x88, 0x00, 0x00, 0x00, 0x03, 0x00, 0xFF, 0x00
};

uint8_t V6MiLightUdpServer::SEARCH_COMMAND[] = {
  0x10, 0x00, 0x00, 0x00
  //, 0x24, 0x02
  //, 0xAE, 0x65, 0x02, 0x39, 0x38, 0x35, 0x62
};

uint8_t V6MiLightUdpServer::SEARCH_RESPONSE[] = {
  0x18, 0x00, 0x00, 0x00, 0x40, 0x02,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // mac address
  0x00, 0x20, 0x39, 0x38, 0x35, 0x62,
  0x31, 0x35, 0x37, 0x62, 0x66, 0x36,
  0x66, 0x63, 0x34, 0x33, 0x33, 0x36,
  0x38, 0x61, 0x36, 0x33, 0x34, 0x36,
  0x37, 0x65, 0x61, 0x33, 0x62, 0x31,
  0x39, 0x64, 0x30, 0x64, 0x01, 0x00,
  0x01,
  0x17, 0x63,  // this is 5987 in hex. specifying a different value seems to
               // cause client to connect on a different port for some commands
  0x00, 0x00, 0x05, 0x00, 0x09, 0x78,
  0x6C, 0x69, 0x6E, 0x6B, 0x5F, 0x64,
  0x65, 0x76, 0x07, 0x5B, 0xCD, 0x15
};

uint8_t V6MiLightUdpServer::OPEN_COMMAND_RESPONSE[] = {
  0x80, 0x00, 0x00, 0x00, 0x15,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // mac address
  0x05, 0x02, 0x00, 0x34, 0x00, 0x00,
  0x00, 0x00 ,0x00 ,0x00, 0x00, 0x00,
  0x00, 0x00, 0x34
};

V6MiLightUdpServer::~V6MiLightUdpServer() {
  V6Session* cur = firstSession;

  while (cur != NULL) {
    V6Session* next = cur->next;
    delete cur;
    cur = next;
  }
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

  V6Session* session = new V6Session(socket.remoteIP(), socket.remotePort(), id);
  session->next = firstSession;
  firstSession = session;

  if (numSessions >= V6_MAX_SESSIONS) {
    V6Session* cur = firstSession;

    for (size_t i = 1; i < V6_MAX_SESSIONS; i++) {
      cur = cur->next;
    }

    delete cur->next;
    cur->next = NULL;
  } else {
    numSessions++;
  }

  return id;
}

void V6MiLightUdpServer::handleSearch() {
  const size_t packetLen = size(SEARCH_RESPONSE);
  uint8_t response[packetLen];
  memcpy(response, SEARCH_RESPONSE, packetLen);
  writeMacAddr(response + 6);

  socket.beginPacket(socket.remoteIP(), socket.remotePort());
  socket.write(response, packetLen);
  socket.endPacket();
}

void V6MiLightUdpServer::handleStartSession() {
  size_t len = size(START_SESSION_RESPONSE);
  uint8_t response[len];
  uint16_t sessionId = beginSession();

  memcpy(response, START_SESSION_RESPONSE, len);
  writeMacAddr(response + 7);

  response[19] = sessionId >> 8;
  response[20] = sessionId & 0xFF;

  sendResponse(sessionId, response, len);
}

bool V6MiLightUdpServer::sendResponse(uint16_t sessionId, uint8_t* responseBuffer, size_t responseSize) {
  V6Session* session = firstSession;

  while (session != NULL) {
    if (session->sessionId == sessionId) {
      break;
    }
    session = session->next;
  }

  if (session == NULL || session->sessionId != sessionId) {
    Serial.print("Received request with untracked session ID: ");
    Serial.println(sessionId);
    return false;
  }

#ifdef MILIGHT_UDP_DEBUG
  printf_P("Sending response to %s:%d\n", session->ipAddr.toString().c_str(), session->port);
#endif

  socket.beginPacket(session->ipAddr, session->port);
  socket.write(responseBuffer, responseSize);
  socket.endPacket();

  return true;
}

bool V6MiLightUdpServer::handleOpenCommand(uint16_t sessionId) {
  size_t len = size(OPEN_COMMAND_RESPONSE);
  uint8_t response[len];
  memcpy(response, OPEN_COMMAND_RESPONSE, len);
  writeMacAddr(response + 5);

  return sendResponse(sessionId, response, len);
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
  printf("Command cmdType: %02X, cmdHeader: %08X, cmdArg: %08X\n", cmdType, cmdHeader, cmdArg);
#endif

  bool handled = false;

  if (cmdHeader == 0) {
    handled = handleOpenCommand(sessionId);
  } else {
    handled = COMMAND_DEMUXER.handleCommand(
      client,
      deviceId,
      group,
      cmdType,
      cmdHeader,
      cmdArg
    );
  }

  if (handled) {
    size_t len = size(COMMAND_RESPONSE);
    memcpy(responseBuffer, COMMAND_RESPONSE, len);
    responseBuffer[6] = sequenceNum;

    sendResponse(sessionId, responseBuffer, len);

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

void V6MiLightUdpServer::handleHeartbeat(uint16_t sessionId) {
  char header[] = { 0xD8, 0x00, 0x00, 0x00, 0x07 };
  memcpy(responseBuffer, header, size(header));
  writeMacAddr(responseBuffer + 5);

  responseBuffer[11] = 0;

  sendResponse(sessionId, responseBuffer, 12);
}

bool V6MiLightUdpServer::matchesPacket(uint8_t* packet1, size_t packet1Len, uint8_t* packet2, size_t packet2Len) {
  return packet2Len >= packet1Len && memcmp(packet1, packet2, packet1Len) == 0;
}

void V6MiLightUdpServer::handlePacket(uint8_t* packet, size_t packetSize) {
#ifdef MILIGHT_UDP_DEBUG
  printf_P("Packet size: %d\n", packetSize);
#endif

  if (MATCHES_PACKET(START_SESSION_COMMAND)) {
    handleStartSession();
  } else if (MATCHES_PACKET(HEARTBEAT_HEADER) || MATCHES_PACKET(HEARTBEAT_HEADER2)) {
    uint16_t sessionId = readInt<uint16_t>(packet+5);
    handleHeartbeat(sessionId);
  } else if (MATCHES_PACKET(SEARCH_COMMAND)) {
    handleSearch();
  } else if (packetSize == 22 && MATCHES_PACKET(COMMAND_HEADER)) {
    uint16_t sessionId = readInt<uint16_t>(packet+5);
    uint8_t sequenceNum = packet[8];
    uint8_t* cmd = packet+10;
    uint8_t group = packet[19];
    uint8_t checksum = packet[21];

#ifdef MILIGHT_UDP_DEBUG
    printf("session: %04X, sequence: %d, group: %d, checksum: %d\n", sessionId, sequenceNum, group, checksum);
#endif

    handleCommand(sessionId, sequenceNum, cmd, group, checksum);
  } else {
    Serial.println(F("Unhandled V6 packet"));
  }
}

void V6MiLightUdpServer::writeMacAddr(uint8_t* packet) {
  memset(packet, 0, 6);
  packet[4] = deviceId >> 8;
  packet[5] = deviceId;
}
