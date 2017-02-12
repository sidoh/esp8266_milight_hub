#include <MiLightUdpServer.h>

MiLightUdpServer::MiLightUdpServer(MiLightClient*& client, uint16_t port, uint16_t deviceId)
  : client(client), 
    port(port),
    deviceId(deviceId),
    lastGroup(0)
{ }

void MiLightUdpServer::begin() {
  socket.begin(this->port);
}

void MiLightUdpServer::handleClient() {
  const size_t packetSize = socket.parsePacket();
  
  if (packetSize) {
    if (packetSize == 3) {
      socket.read(packetBuffer, packetSize);
      handleCommand(packetBuffer[0], packetBuffer[1]);
    } else {
      Serial.print("Error, unexpected packet length (should always be 3, was: ");
      Serial.println(packetSize);
    }
  }
}

void MiLightUdpServer::handleCommand(uint8_t command, uint8_t commandArg) {
  if (command >= UDP_GROUP_1_ON && command <= UDP_GROUP_4_OFF) {
    const MiLightStatus status = (command % 2) == 1 ? ON : OFF;
    const uint8_t groupId = (command - UDP_GROUP_1_ON + 2)/2;
    
    client->updateStatus(deviceId, groupId, status);
    
    this->lastGroup = groupId;
  } else if (command >= UDP_GROUP_ALL_WHITE && command <= UDP_GROUP_4_WHITE) {
    const uint8_t groupId = (command - UDP_GROUP_ALL_WHITE)/2;
    client->updateColorWhite(deviceId, groupId);
    this->lastGroup = groupId;
  } else {
    // Group on/off
    switch (command) {
      case UDP_ALL_ON:
        client->allOn(deviceId);
        break;
      
      case UDP_ALL_OFF:
        client->allOff(deviceId);
        break;
      
      case UDP_COLOR:
        client->updateColorRaw(deviceId, this->lastGroup, commandArg);
        break;
        
      case UDP_DISCO_MODE:
        pressButton(this->lastGroup, DISCO_MODE);
        break;
        
      case UDP_SPEED_DOWN:
        pressButton(this->lastGroup, SPEED_DOWN);
        break;
        
      case UDP_SPEED_UP:
        pressButton(this->lastGroup, SPEED_UP);
        break;
        
      case UDP_BRIGHTNESS:
        // map [2, 27] --> [0, 100]
        client->updateBrightness(
          deviceId, 
          this->lastGroup, 
          round(((commandArg - 2) / 25.0)*100)
        );
        break;
        
      default:
        Serial.print("MiLightUdpServer - Unhandled command: ");
        Serial.println(command);
    }
  }
}

void MiLightUdpServer::pressButton(uint8_t group, MiLightButton button) {
  client->write(deviceId, 0, 0, group, button);
}  