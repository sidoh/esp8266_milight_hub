#include <V5MiLightUdpServer.h>

void V5MiLightUdpServer::handlePacket(uint8_t* packet, size_t packetSize) {
  if (packetSize == 2 || packetSize == 3) {
    handleCommand(packet[0], packet[1]);
  } else {
    Serial.print(F("V5MilightUdpServer: unexpected packet length. Should always be 2-3, was: "));
    Serial.println(packetSize);  
  }
}

void V5MiLightUdpServer::handleCommand(uint8_t command, uint8_t commandArg) {
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
          Serial.print(F("V5MiLightUdpServer - Unhandled command: "));
          Serial.println(command);
        }
    }
  }
}

void V5MiLightUdpServer::pressButton(uint8_t button) {
  client->command(button, 0);
}  

uint8_t V5MiLightUdpServer::cctCommandIdToGroup(uint8_t command) {
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
  
MiLightStatus V5MiLightUdpServer::cctCommandToStatus(uint8_t command) {
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