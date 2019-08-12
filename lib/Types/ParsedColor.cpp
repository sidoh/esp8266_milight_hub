#include <ParsedColor.h>
#include <RGBConverter.h>
#include <TokenIterator.h>
#include <GroupStateField.h>
#include <IntParsing.h>

ParsedColor ParsedColor::fromRgb(uint16_t r, uint16_t g, uint16_t b) {
  double hsv[3];
  RGBConverter converter;
  converter.rgbToHsv(r, g, b, hsv);

  uint16_t hue = round(hsv[0]*360);
  uint8_t saturation = round(hsv[1]*100);

  return ParsedColor{
    .success = true,
    .hue = hue,
    .r = r,
    .g = g,
    .b = b,
    .saturation = saturation
  };
}

ParsedColor ParsedColor::fromJson(JsonVariant json) {
  uint16_t r, g, b;

  if (json.is<JsonObject>()) {
    JsonObject color = json.as<JsonObject>();

    r = color["r"];
    g = color["g"];
    b = color["b"];
  } else if (json.is<const char*>()) {
    const char* colorStr = json.as<const char*>();
    const size_t len = strlen(colorStr);

    if (colorStr[0] == '#' && len == 7) {
      uint8_t parsedHex[3];
      hexStrToBytes<uint8_t>(colorStr+1, len-1, parsedHex, 3);

      r = parsedHex[0];
      g = parsedHex[1];
      b = parsedHex[2];
    } else {
      char colorCStr[len+1];
      uint8_t parsedRgbColors[3] = {0, 0, 0};

      strcpy(colorCStr, colorStr);
      TokenIterator colorValueItr(colorCStr, len, ',');

      for (size_t i = 0; i < 3 && colorValueItr.hasNext(); ++i) {
        parsedRgbColors[i] = atoi(colorValueItr.nextToken());
      }

      r = parsedRgbColors[0];
      g = parsedRgbColors[1];
      b = parsedRgbColors[2];
    }
  } else {
    Serial.println(F("GroupState::parseJsonColor - unknown format for color"));
    return ParsedColor{ .success = false };
  }

  return ParsedColor::fromRgb(r, g, b);
}