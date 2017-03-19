#include <MiLightUdpServer.h>

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
    if (packetSize >= 2 && packetSize <= 3) {
      socket.read(packetBuffer, packetSize);
      
#ifdef MILIGHT_UDP_DEBUG
      Serial.print("Handling command: ");
      Serial.print(String(packetBuffer[0], HEX));
      Serial.print(" ");
      Serial.println(String(packetBuffer[1], HEX));
#endif
      
      handleCommand(packetBuffer[0], packetBuffer[1]);
    } else {
      Serial.print("Error, unexpected packet length (should always be 2-3, was: ");
      Serial.println(packetSize);
    }
  }
}

void MiLightUdpServer::handleCommand(uint8_t command, uint8_t commandArg) {
  if (command >= UDP_RGBW_GROUP_1_ON && command <= UDP_RGBW_GROUP_4_OFF) {
    const MiLightStatus status = (command % 2) == 1 ? ON : OFF;
    const uint8_t groupId = (command - UDP_RGBW_GROUP_1_ON + 2)/2;
    
    client->prepare(MilightRgbwConfig, deviceId, groupId);
    client->updateStatus(status);
    
    this->lastGroup = groupId;
  } else if (command >= UDP_RGBW_GROUP_ALL_WHITE && command <= UDP_RGBW_GROUP_4_WHITE) {
    const uint8_t groupId = (command - UDP_RGBW_GROUP_ALL_WHITE)/2;
    client->prepare(MilightRgbwConfig, deviceId, groupId);
    client->updateColorWhite();
    this->lastGroup = groupId;
  } else if (uint8_t cctGroup = cctCommandIdToGroup(command)) {
    client->prepare(MilightCctConfig, deviceId, cctGroup);
    client->updateStatus(cctCommandToStatus(command));
    this->lastGroup = cctGroup;
  }
  else {
    client->prepare(MilightRgbwConfig, deviceId, lastGroup);
    bool handled = true;
    
    switch (command) {
      case UDP_RGBW_ALL_ON:
        client->updateStatus(ON, 0);
        break;
      
      case UDP_RGBW_ALL_OFF:
        client->updateStatus(OFF, 0);
        break;
      
      case UDP_RGBW_COLOR:
        // UDP color is shifted by 0xC8 from 2.4 GHz color, and the spectrum is
        // flipped (R->B->G instead of R->G->B)
        client->updateColorRaw(0xFF-(commandArg + 0x35));
        break;
        
      case UDP_RGBW_DISCO_MODE:
        pressButton(RGBW_DISCO_MODE);
        break;
        
      case UDP_RGBW_SPEED_DOWN:
        pressButton(RGBW_SPEED_DOWN);
        break;
        
      case UDP_RGBW_SPEED_UP:
        pressButton(RGBW_SPEED_UP);
        break;
        
      case UDP_RGBW_BRIGHTNESS:
        // map [2, 27] --> [0, 100]
        client->updateBrightness(
          round(((commandArg - 2) / 25.0)*100)
        );
        break;
        
      default:
        handled = false;
    }
    
    client->prepare(MilightCctConfig);
    
    switch(command) {
      case UDP_CCT_BRIGHTNESS_DOWN:
        client->decreaseBrightness();
        break;
        
      case UDP_CCT_BRIGHTNESS_UP:
        client->increaseBrightness();
        break;
        
      case UDP_CCT_TEMPERATURE_DOWN:
        client->decreaseTemperature();
        break;
        
      case UDP_CCT_TEMPERATURE_UP:
        client->increaseTemperature();
        break;
        
      default:
        if (!handled) {
          Serial.print("MiLightUdpServer - Unhandled command: ");
          Serial.println(command);
        }
    }
  }
}

void MiLightUdpServer::pressButton(uint8_t button) {
  client->command(button, 0);
}  

uint8_t MiLightUdpServer::cctCommandIdToGroup(uint8_t command) {
  switch (command) {
    case UDP_CCT_GROUP_1_ON:
    case UDP_CCT_GROUP_1_OFF:
      return 1;
    case UDP_CCT_GROUP_2_ON:
    case UDP_CCT_GROUP_2_OFF:
      return 2;
    case UDP_CCT_GROUP_3_ON:
    case UDP_CCT_GROUP_3_OFF:
      return 3;
    case UDP_CCT_GROUP_4_ON:
    case UDP_CCT_GROUP_4_OFF:
      return 4;
  }
  
  return 0;
}  
  
MiLightStatus MiLightUdpServer::cctCommandToStatus(uint8_t command) {
  switch (command) {
    case UDP_CCT_GROUP_1_ON:
    case UDP_CCT_GROUP_2_ON:
    case UDP_CCT_GROUP_3_ON:
    case UDP_CCT_GROUP_4_ON:
      return ON;
    case UDP_CCT_GROUP_1_OFF:
    case UDP_CCT_GROUP_2_OFF:
    case UDP_CCT_GROUP_3_OFF:
    case UDP_CCT_GROUP_4_OFF:
      return OFF;
  }
}