#include <Settings.h>
#include <ArduinoJson.h>
#include <FS.h>

void Settings::deserialize(Settings& settings, String json) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& parsedSettings = jsonBuffer.parseObject(json);
  deserialize(settings, parsedSettings);
}

void Settings::deserialize(Settings& settings, JsonObject& parsedSettings) {
  if (parsedSettings.success()) {
    settings.adminUsername = parsedSettings.get<String>("admin_username");
    settings.adminPassword = parsedSettings.get<String>("admin_password");
    settings.cePin = parsedSettings["ce_pin"];
    settings.csnPin = parsedSettings["csn_pin"];
    
    JsonArray& arr = parsedSettings["device_ids"];
    settings.updateDeviceIds(arr);
  }
}

void Settings::updateDeviceIds(JsonArray& arr) {
  if (arr.success()) {
    if (this->deviceIds) {
      delete this->deviceIds;
    }
    
    this->deviceIds = new uint16_t[arr.size()];
    this->numDeviceIds = arr.size();
    arr.copyTo(this->deviceIds, arr.size());
  }
}

void Settings::patch(JsonObject& parsedSettings) {
  if (parsedSettings.success()) {
    if (parsedSettings.containsKey("admin_username")) {
      this->adminUsername = parsedSettings.get<String>("admin_username");
    }
    if (parsedSettings.containsKey("admin_password")) {
      this->adminPassword = parsedSettings.get<String>("admin_password");
    }
    if (parsedSettings.containsKey("ce_pin")) {
      this->cePin = parsedSettings["ce_pin"];
    }
    if (parsedSettings.containsKey("csn_pin")) {
      this->csnPin = parsedSettings["csn_pin"];
    }
    if (parsedSettings.containsKey("device_ids")) {
      JsonArray& arr = parsedSettings["device_ids"];
      updateDeviceIds(arr);
    }
  }
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
  
  if (this->deviceIds) {
    JsonArray& arr = jsonBuffer.createArray();
    arr.copyFrom(this->deviceIds, this->numDeviceIds);
    root["device_ids"] = arr;
  }
  
  if (prettyPrint) {
    root.prettyPrintTo(stream);
  } else {
    root.printTo(stream);
  }
}