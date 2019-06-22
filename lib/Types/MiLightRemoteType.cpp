#include <MiLightRemoteType.h>
#include <Arduino.h>

const MiLightRemoteType MiLightRemoteTypeHelpers::remoteTypeFromString(const String& type) {
  if (type.equalsIgnoreCase("rgbw") || type.equalsIgnoreCase("fut096")) {
    return REMOTE_TYPE_RGBW;
  }

  if (type.equalsIgnoreCase("cct") || type.equalsIgnoreCase("fut007")) {
    return REMOTE_TYPE_CCT;
  }

  if (type.equalsIgnoreCase("rgb_cct") || type.equalsIgnoreCase("fut092")) {
    return REMOTE_TYPE_RGB_CCT;
  }

  if (type.equalsIgnoreCase("fut089")) {
    return REMOTE_TYPE_FUT089;
  }

  if (type.equalsIgnoreCase("rgb") || type.equalsIgnoreCase("fut098")) {
    return REMOTE_TYPE_RGB;
  }

  if (type.equalsIgnoreCase("v2_cct") || type.equalsIgnoreCase("fut091")) {
    return REMOTE_TYPE_FUT091;
  }

  Serial.print(F("remoteTypeFromString: ERROR - tried to fetch remote config for type: "));
  Serial.println(type);

  return REMOTE_TYPE_UNKNOWN;
}

const String MiLightRemoteTypeHelpers::remoteTypeToString(const MiLightRemoteType type) {
  switch (type) {
    case REMOTE_TYPE_RGBW:
      return "rgbw";
    case REMOTE_TYPE_CCT:
      return "cct";
    case REMOTE_TYPE_RGB_CCT:
      return "rgb_cct";
    case REMOTE_TYPE_FUT089:
      return "fut089";
    case REMOTE_TYPE_RGB:
      return "rgb";
    case REMOTE_TYPE_FUT091:
      return "fut091";
    default:
      Serial.print(F("remoteTypeToString: ERROR - tried to fetch remote config name for unknown type: "));
      Serial.println(type);
      return "unknown";
  }
}