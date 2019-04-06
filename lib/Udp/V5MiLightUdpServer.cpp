#include <V5MiLightUdpServer.h>
#include <CctPacketFormatter.h>

void V5MiLightUdpServer::handlePacket(uint8_t* packet, size_t packetSize) {
  if (packetSize == 2 || packetSize == 3) {
    handleCommand(packet[0], packet[1]);
  } else {
    Serial.print(F("V5MilightUdpServer: unexpected packet length. Should always be 2-3, was: "));
    Serial.println(packetSize);
  }
}

void V5MiLightUdpServer::handleCommand(uint8_t command, uint8_t commandArg) {
  // On/off for RGBW
  if (command >= UDP_RGBW_GROUP_1_ON && command <= UDP_RGBW_GROUP_4_OFF) {
    const MiLightStatus status = (command % 2) == 1 ? ON : OFF;
    const uint8_t groupId = (command - UDP_RGBW_GROUP_1_ON + 2)/2;

    client->prepare(&FUT096Config, deviceId, groupId);
    client->updateStatus(status);

    this->lastGroup = groupId;
  // Command set_white for RGBW
 } else if (command == UDP_RGBW_GROUP_ALL_WHITE || command == UDP_RGBW_GROUP_1_WHITE || command == UDP_RGBW_GROUP_2_WHITE || command == UDP_RGBW_GROUP_3_WHITE || command == UDP_RGBW_GROUP_4_WHITE) {
    const uint8_t groupId = (command - UDP_RGBW_GROUP_ALL_WHITE)/2;
    client->prepare(&FUT096Config, deviceId, groupId);
    client->updateColorWhite();

    this->lastGroup = groupId;
  // Set night_mode for RGBW
 } else if (command == UDP_RGBW_GROUP_ALL_NIGHT || command == UDP_RGBW_GROUP_1_NIGHT || command == UDP_RGBW_GROUP_2_NIGHT || command == UDP_RGBW_GROUP_3_NIGHT || command == UDP_RGBW_GROUP_4_NIGHT) {
    uint8_t groupId = (command - UDP_RGBW_GROUP_1_NIGHT + 2)/2;
    if (command == UDP_RGBW_GROUP_ALL_NIGHT) {
      groupId = 0;
    }

    client->prepare(&FUT096Config, deviceId, groupId);
    client->enableNightMode();

    this->lastGroup = groupId;
 } else {
    client->prepare(&FUT096Config, deviceId, lastGroup);
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
        client->nextMode();
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

    if (handled) {
      return;
    }

    uint8_t onOffGroup = CctPacketFormatter::cctCommandIdToGroup(command);

    if (onOffGroup != 255) {
      client->prepare(&FUT007Config, deviceId, onOffGroup);
      // Night mode commands are same as off commands with MSB set
      if ((command & 0x80) == 0x80) {
        client->enableNightMode();
      } else {
        client->updateStatus(CctPacketFormatter::cctCommandToStatus(command));
      }
      return;
    }

    client->prepare(&FUT007Config, deviceId, lastGroup);

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

      case UDP_CCT_NIGHT_MODE:
        client->enableNightMode();
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
