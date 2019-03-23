#include <Arduino.h>

#ifndef _ABOUT_STRING_HELPER_H
#define _ABOUT_STRING_HELPER_H

class AboutStringHelper {
public:
  static String generateAboutString(bool abbreviated = false);
};

#endif