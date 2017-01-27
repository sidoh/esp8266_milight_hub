#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <stdlib.h>

#include <PL1167_nRF24.h>
#include <MiLightRadio.h>
#include <MiLightClient.h>
#include <ESP8266WebServer.h>

#define CE_PIN D0 
#define CSN_PIN D8

RF24 radio(CE_PIN, CSN_PIN);
PL1167_nRF24 prf(radio);
MiLightRadio mlr(prf);
MiLightClient milightClient(mlr);

WiFiManager wifiManager;
ESP8266WebServer server(80);

template <typename T>
const T strToHex(const String& s) {
  T value = 0;
  uint32_t base = 1;
  
  for (int i = s.length() - 1; i >= 0; i--) {
    const char c = s.charAt(i);
    
    if (c >= '0' && c <= '9') {
      value += ((c - '0') * base);
    } else if (c >= 'a' && c <= 'f') {
      value += ((c - 'a' + 10) * base);
    } else if (c >= 'A' && c <= 'F') {
      value += ((c - 'A' + 10) * base);
    } else {
      break;
    }
    
    base <<= 4;
  }
  
  return value;
}

void handleUpdateGateway() {
  DynamicJsonBuffer buffer;
  JsonObject& request = buffer.parse(server.arg("plain"));
  
  const String gatewayIdStr = request["gateway_id"];
  const uint8_t groupId = request["group_id"];
  uint16_t gatewayId;
  
  if (gatewayIdStr.startsWith("0x")) {
    gatewayId = strToHex<uint16_t>(gatewayIdStr.substring(2));
  } else {
    gatewayId = request["gateway_id"];
  }
  
  if (request.containsKey("status")) {
    MiLightStatus status = (request.get<String>("status") == "on") ? ON : OFF;
    milightClient.updateStatus(gatewayId, groupId, status);
  }
  
  if (request.containsKey("hue")) {
    milightClient.updateColor(gatewayId, groupId, request["hue"]);
  }
  
  if (request.containsKey("level")) {
    milightClient.updateBrightness(gatewayId, groupId, request["level"]);
  }
  
  if (request.containsKey("command")) {
    if (request["command"] == "set_white") {
      milightClient.updateColorWhite(gatewayId, groupId);
    }
    
    if (request["command"] == "all_on") {
      milightClient.allOn(gatewayId);
    }
    
    if (request["command"] == "all_off") {
      milightClient.allOff(gatewayId);
    }
    
    if (request["command"] == "unpair") {
      milightClient.unpair(gatewayId, groupId);
    }
    
    if (request["command"] == "pair") {
      milightClient.pair(gatewayId, groupId);
    }
  }
  
  server.send(200, "application/json", "true");
}

void handleListenGateway() {
  while (!mlr.available()) {
    yield();
  }
  
  MiLightPacket packet;
  milightClient.read(packet);
  
  String response = "Packet received (";
  response += String(sizeof(packet)) + " bytes)";
  response += ":\n";
  response += "Request type : " + String(packet.deviceType, HEX) + "\n";
  response += "Device ID    : " + String(packet.deviceId, HEX) + "\n";
  response += "Color        : " + String(packet.color, HEX) + "\n";
  response += "Brightness   : " + String(packet.brightness, HEX) + "\n";
  response += "Group ID     : " + String(packet.groupId, HEX) + "\n";
  response += "Button       : " + String(packet.button, HEX) + "\n";
  response += "Sequence Num : " + String(packet.sequenceNum, HEX) + "\n";
  response += "\n\n";
  
  server.send(200, "text/plain", response);
}

void setup() {
  Serial.begin(9600);
  wifiManager.autoConnect();
  mlr.begin();
  
  server.on("/gateway", HTTP_PUT, handleUpdateGateway);
  server.on("/gateway", HTTP_GET, handleListenGateway);
  server.on("/update", HTTP_POST, 
    [](){
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      ESP.restart();
    },
    [](){
      HTTPUpload& upload = server.upload();
      if(upload.status == UPLOAD_FILE_START){
        WiFiUDP::stopAll();
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
        } else {
          Update.printError(Serial);
        }
      }
      yield();
    }
  );
  
  server.begin();
}

void loop() {
  server.handleClient();
}
