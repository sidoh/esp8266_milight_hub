#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <fs.h>

#include <PL1167_nRF24.h>
#include <MiLightRadio.h>
#include <MiLightClient.h>
#include <WebServer.h>
#include <IntParsing.h>

#define CE_PIN D0 
#define CSN_PIN D8

#define WEB_INDEX_FILENAME "/index.html"

RF24 radio(CE_PIN, CSN_PIN);
PL1167_nRF24 prf(radio);
MiLightRadio mlr(prf);
MiLightClient milightClient(mlr);

WiFiManager wifiManager;
WebServer server(80);
File updateFile;

void handleUpdateGateway() {
  DynamicJsonBuffer buffer;
  JsonObject& request = buffer.parse(server.arg("plain"));
  
  const uint16_t gatewayId = parseInt<uint16_t>(server.arg("gateway_id"));
  
  if (request.containsKey("status")) {
    if (request["status"] == "on") {
      milightClient.allOn(gatewayId);
    } else if (request["status"] == "off") {
      milightClient.allOff(gatewayId);
    }
  }
  
  server.send(200, "application/json", "true");
}

void handleUpdateGroup() {
  DynamicJsonBuffer buffer;
  JsonObject& request = buffer.parse(server.arg("plain"));
  
  const uint16_t gatewayId = parseInt<uint16_t>(server.arg("gateway_id"));
  const uint8_t groupId = server.arg("group_id").toInt();
  
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
    if (!server.clientConnected()) {
      return;
    }
    
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

void serveFile(const String& file, const char* contentType = "text/html") {
  if (SPIFFS.exists(file)) {
    File f = SPIFFS.open(file, "r");
    server.send(200, "text/html", f.readString());
    f.close();
  } else {
    server.send(404);
  }
}

void handleIndex() {
  serveFile(WEB_INDEX_FILENAME);
}

void handleWebUpdate() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    updateFile = SPIFFS.open(WEB_INDEX_FILENAME, "w");
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if (updateFile.write(upload.buf, upload.currentSize)) {
      Serial.println("Error updating web file");
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    updateFile.close();
  }
  
  yield();
}

void onWebUpdated() {
  server.sendHeader("Location", "/");
  server.send(302);
}

void setup() {
  Serial.begin(9600);
  wifiManager.autoConnect();
  mlr.begin();
  SPIFFS.begin();
  
  server.on("/", HTTP_GET, handleIndex);
  server.on("/gateway_traffic", HTTP_GET, handleListenGateway);
  server.onPattern("/gateway/:gateway_id/:group_id", HTTP_PUT, handleUpdateGroup);
  server.onPattern("/gateway/:gateway_id", HTTP_PUT, handleUpdateGateway);
  server.on("/web", HTTP_POST, onWebUpdated, handleWebUpdate);
  server.on("/firmware", HTTP_POST, 
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
