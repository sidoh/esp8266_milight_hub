#include <MiLightUdpServer.h>
#include <V5MiLightUdpServer.h>

MiLightUdpServer::MiLightUdpServer(MiLightClient*& client, uint16_t port, uint16_t deviceId)
  : client(client), 
    port(port),
    deviceId(deviceId),
    lastGroup(0)
{ }

MiLightUdpServer::~MiLightUdpServer() {
  stop();
}

void MiLightUdpServer::begin() {
  socket.begin(this->port);
}

void MiLightUdpServer::stop() {
  socket.stop();
}

void MiLightUdpServer::handleClient() {
  const size_t packetSize = socket.parsePacket();
  
  if (packetSize) {
    socket.read(packetBuffer, packetSize);
    
#ifdef MILIGHT_UDP_DEBUG
    Serial.print("Handling packet: ");
    for (size_t i = 0; i < packetSize; i++) {
      Serial.printf("%02X ", packetBuffer[0])
    }
    Serial.println();
#endif
    
    size_t responseSize = handlePacket(packetBuffer, packetSize, responseBuffer);
    
    if (responseSize > 0) {
      socket.write(responseBuffer, responseSize);
    }
  }
}

MiLightUdpServer* MiLightUdpServer::fromVersion(uint8_t version, MiLightClient*& client, uint16_t port, uint16_t deviceId) {
  if (version == 0 || version == 5) {
    return new V5MiLightUdpServer(client, port, deviceId);
  }
  
  return NULL;
}