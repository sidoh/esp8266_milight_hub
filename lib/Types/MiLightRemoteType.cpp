#include <MiLightRemoteType.h>
#include <Arduino.h>

static const char* REMOTE_NAME_RGBW    = "rgbw";
static const char* REMOTE_NAME_CCT     = "cct";
static const char* REMOTE_NAME_RGB_CCT = "rgb_cct";
static const char* REMOTE_NAME_FUT089  = "fut089";
static const char* REMOTE_NAME_RGB     = "rgb";
static const char* REMOTE_NAME_FUT091  = "fut091";
static const char* REMOTE_NAME_FUT020  = "fut020";

const MiLightRemoteType MiLightRemoteTypeHelpers::remoteTypeFromString(const String& type) {
  if (type.equalsIgnoreCase(REMOTE_NAME_RGBW) || type.equalsIgnoreCase("fut096")) {
    return REMOTE_TYPE_RGBW;
  }

  if (type.equalsIgnoreCase(REMOTE_NAME_CCT) || type.equalsIgnoreCase("fut007")) {
    return REMOTE_TYPE_CCT;
  }

  if (type.equalsIgnoreCase(REMOTE_NAME_RGB_CCT) || type.equalsIgnoreCase("fut092")) {
    return REMOTE_TYPE_RGB_CCT;
  }

  if (type.equalsIgnoreCase(REMOTE_NAME_FUT089)) {
    return REMOTE_TYPE_FUT089;
  }

  if (type.equalsIgnoreCase(REMOTE_NAME_RGB) || type.equalsIgnoreCase("fut098")) {
    return REMOTE_TYPE_RGB;
  }

  if (type.equalsIgnoreCase("v2_cct") || type.equalsIgnoreCase(REMOTE_NAME_FUT091)) {
    return REMOTE_TYPE_FUT091;
  }

  if (type.equalsIgnoreCase(REMOTE_NAME_FUT020)) {
    return REMOTE_TYPE_FUT020;
  }

  Serial.print(F("remoteTypeFromString: ERROR - tried to fetch remote config for type: "));
  Serial.println(type);

  return REMOTE_TYPE_UNKNOWN;
}

const String MiLightRemoteTypeHelpers::remoteTypeToString(const MiLightRemoteType type) {
  switch (type) {
    case REMOTE_TYPE_RGBW:
      return REMOTE_NAME_RGBW;
    case REMOTE_TYPE_CCT:
      return REMOTE_NAME_CCT;
    case REMOTE_TYPE_RGB_CCT:
      return REMOTE_NAME_RGB_CCT;
    case REMOTE_TYPE_FUT089:
      return REMOTE_NAME_FUT089;
    case REMOTE_TYPE_RGB:
      return REMOTE_NAME_RGB;
    case REMOTE_TYPE_FUT091:
      return REMOTE_NAME_FUT091;
    case REMOTE_TYPE_FUT020:
      return REMOTE_NAME_FUT020;
    default:
      Serial.print(F("remoteTypeToString: ERROR - tried to fetch remote config name for unknown type: "));
      Serial.println(type);
      return "unknown";
  }
}