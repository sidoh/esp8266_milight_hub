#include <Arduino.h>
#include <ArduinoJson.h>

#ifndef _ABOUT_STRING_HELPER_H
#define _ABOUT_STRING_HELPER_H

class AboutHelper {
public:
  static String generateAboutString(bool abbreviated = false);
  static void generateAboutObject(JsonDocument& obj, bool abbreviated = false);
};

#endif