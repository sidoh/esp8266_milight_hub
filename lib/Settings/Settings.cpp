#include <Settings.h>
#include <ArduinoJson.h>
#include <FS.h>

void Settings::deserialize(Settings& settings, String json) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& parsedSettings = jsonBuffer.parseObject(json);
    
  settings.adminUsername = parsedSettings.get<String>("admin_username");
  settings.adminPassword = parsedSettings.get<String>("admin_password");
  settings.cePin = parsedSettings["ce_pin"];
  settings.csnPin = parsedSettings["csn_pin"];
}

void Settings::load(Settings& settings) {
  if (SPIFFS.exists(SETTINGS_FILE)) {
    File f = SPIFFS.open(SETTINGS_FILE, "r");
    String settingsContents = f.readStringUntil(SETTINGS_TERMINATOR);
    f.close();
    
    deserialize(settings, settingsContents);
  } else {
    settings.save();
  }
}

String Settings::toJson(const bool prettyPrint) {
  String buffer = "";
  StringStream s(buffer);
  serialize(s, prettyPrint);
  return buffer;
}

void Settings::save() {
  File f = SPIFFS.open(SETTINGS_FILE, "w");
  
  if (!f) {
    Serial.println("Opening settings file failed");
  } else {
    serialize(f);
    f.close();
  }
}

void Settings::serialize(Stream& stream, const bool prettyPrint) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  
  root["admin_username"] = this->adminUsername;
  root["admin_password"] = this->adminPassword;
  root["ce_pin"] = this->cePin;
  root["csn_pin"] = this->csnPin;
  
  if (prettyPrint) {
    root.prettyPrintTo(stream);
  } else {
    root.printTo(stream);
  }
}