//
// Created by chris on 9/17/2023.
//

#ifndef ESP8266_MILIGHT_HUB_PROJECTWIFI_H
#define ESP8266_MILIGHT_HUB_PROJECTWIFI_H

#if __has_include(<wifi_credentials.h>)
#include <wifi_credentials.h>
#endif

#if defined(ESPMH_WIFI_SSID) && defined(ESPMH_WIFI_PASSWORD)
#define ESPMH_SETUP_WIFI(settings) { \
  Serial.println(F(">>> ESPMH_WIFI_SETUP() <<<"));\
  if (settings.wifiStaticIP.length() > 0) { \
    Serial.printf_P(PSTR("Configuring static IP...%s\n"), settings.wifiStaticIP.c_str()); \
    IPAddress _ip, _subnet, _gw;\
    _ip.fromString(settings.wifiStaticIP);\
    _subnet.fromString(settings.wifiStaticIPNetmask);\
    _gw.fromString(settings.wifiStaticIPGateway);\
    WiFi.config(_ip, _gw, _subnet);\
  };\
  WiFi.begin(ESPMH_WIFI_SSID, ESPMH_WIFI_PASSWORD); \
  Serial.printf_P(PSTR("Connecting to %s...\n"), ESPMH_WIFI_SSID); \
  while (WiFi.status() != WL_CONNECTED) {\
    delay(500);\
    Serial.print(".");\
  }\
  Serial.printf_P(PSTR("Connected to: %s with IP: "), ESPMH_WIFI_SSID); \
  Serial.println(WiFi.localIP());                  \
}
#else
#define ESPMH_SETUP_WIFI(settings) { }
#endif

#endif //ESP8266_MILIGHT_HUB_PROJECTWIFI_H
