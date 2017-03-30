#include <Settings.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <IntParsing.h>
#include <algorithm>
  
bool Settings::hasAuthSettings() {
  return adminUsername.length() > 0 && adminPassword.length() > 0;
}

bool Settings::isAutoRestartEnabled() {
  return _autoRestartPeriod > 0;
}

size_t Settings::getAutoRestartPeriod() {
  if (_autoRestartPeriod == 0) {
    return 0;
  }
  
  return std::min(_autoRestartPeriod, static_cast<size_t>(MINIMUM_RESTART_PERIOD));
}

void Settings::deserialize(Settings& settings, String json) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& parsedSettings = jsonBuffer.parseObject(json);
  deserialize(settings, parsedSettings);
}

void Settings::deserialize(Settings& settings, JsonObject& parsedSettings) {
  if (parsedSettings.success()) {
    if (parsedSettings.containsKey("admin_username")) {
      settings.adminUsername = parsedSettings.get<String>("admin_username");
    }
    
    if (parsedSettings.containsKey("admin_password")) {
      settings.adminPassword = parsedSettings.get<String>("admin_password");
    }
    
    if (parsedSettings.containsKey("ce_pin")) {
      settings.cePin = parsedSettings["ce_pin"];
    }
    
    if (parsedSettings.containsKey("csn_pin")) {
      settings.csnPin = parsedSettings["csn_pin"];
    }
    
    if (parsedSettings.containsKey("packet_repeats")) {
      settings.packetRepeats = parsedSettings["packet_repeats"];
    }
    
    if (parsedSettings.containsKey("http_repeat_factor")) {
      settings.httpRepeatFactor = parsedSettings["http_repeat_factor"];
    }
    
    if (parsedSettings.containsKey("auto_restart_period")) {
      settings._autoRestartPeriod = parsedSettings["auto_restart_period"];
    }
    
    JsonArray& arr = parsedSettings["device_ids"];
    settings.updateDeviceIds(arr);
    
    JsonArray& gatewayArr = parsedSettings["gateway_configs"];
    settings.updateGatewayConfigs(gatewayArr);
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

void Settings::updateGatewayConfigs(JsonArray& arr) {
  if (arr.success()) {
    if (this->gatewayConfigs) {
      delete[] this->gatewayConfigs;
    }
    
    this->gatewayConfigs = new GatewayConfig*[arr.size()];
    this->numGatewayConfigs = arr.size();
    
    for (size_t i = 0; i < arr.size(); i++) {
      JsonArray& params = arr[i];
      
      if (params.success() && params.size() == 3) {
        this->gatewayConfigs[i] = new GatewayConfig(parseInt<uint16_t>(params[0]), params[1], params[2]);
      } else {
        Serial.print("Settings - skipped parsing gateway ports settings for element #");
        Serial.println(i);
      }
    }
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
    if (parsedSettings.containsKey("packet_repeats")) {
      this->packetRepeats = parsedSettings["packet_repeats"];
    }
    if (parsedSettings.containsKey("http_repeat_factor")) {
      this->httpRepeatFactor = parsedSettings["http_repeat_factor"];
    }
    if (parsedSettings.containsKey("auto_restart_period")) {
      this->_autoRestartPeriod = parsedSettings["auto_restart_period"];
    }
    if (parsedSettings.containsKey("device_ids")) {
      JsonArray& arr = parsedSettings["device_ids"];
      updateDeviceIds(arr);
    }
    if (parsedSettings.containsKey("gateway_configs")) {
      JsonArray& arr = parsedSettings["gateway_configs"];
      updateGatewayConfigs(arr);
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
  root["packet_repeats"] = this->packetRepeats;
  root["http_repeat_factor"] = this->httpRepeatFactor;
  root["auto_restart_period"] = this->_autoRestartPeriod;
  
  if (this->deviceIds) {
    JsonArray& arr = jsonBuffer.createArray();
    arr.copyFrom(this->deviceIds, this->numDeviceIds);
    root["device_ids"] = arr;
  }
  
  if (this->gatewayConfigs) {
    JsonArray& arr = jsonBuffer.createArray();
    for (size_t i = 0; i < this->numGatewayConfigs; i++) {
      JsonArray& elmt = jsonBuffer.createArray();
      elmt.add(this->gatewayConfigs[i]->deviceId);
      elmt.add(this->gatewayConfigs[i]->port);
      elmt.add(this->gatewayConfigs[i]->protocolVersion);
      arr.add(elmt);
    }
    
    root["gateway_configs"] = arr;
  }
  
  if (prettyPrint) {
    root.prettyPrintTo(stream);
  } else {
    root.printTo(stream);
  }
}