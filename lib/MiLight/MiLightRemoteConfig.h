#include <MiLightRadioConfig.h>
#include <PacketFormatter.h>

#include <RgbwPacketFormatter.h>
#include <RgbPacketFormatter.h>
#include <RgbCctPacketFormatter.h>
#include <CctPacketFormatter.h>
#include <FUT089PacketFormatter.h>
#include <PacketFormatter.h>

#ifndef _MILIGHT_REMOTE_CONFIG_H
#define _MILIGHT_REMOTE_CONFIG_H

class MiLightRemoteConfig {
public:
  MiLightRemoteConfig(
    PacketFormatter* packetFormatter,
    MiLightRadioConfig& radioConfig,
    const MiLightRemoteType type,
    const String name
  ) : packetFormatter(packetFormatter),
      radioConfig(radioConfig),
      type(type),
      name(name)
  { }

  PacketFormatter* const packetFormatter;
  const MiLightRadioConfig& radioConfig;
  const MiLightRemoteType type;
  const String name;

  static const MiLightRemoteConfig* fromType(MiLightRemoteType type);
  static const MiLightRemoteConfig* fromType(const String& type);
  static const MiLightRemoteConfig* fromReceivedPacket(const MiLightRadioConfig& radioConfig, const uint8_t* packet, const size_t len);

  static const size_t NUM_REMOTES = 5;
  static const MiLightRemoteConfig* ALL_REMOTES[NUM_REMOTES];
};

static const MiLightRemoteConfig FUT096Config( //rgbw
  new RgbwPacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[0],
  REMOTE_TYPE_RGBW,
  "rgbw"
);

static const MiLightRemoteConfig FUT091Config( //cct
  new CctPacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[1],
  REMOTE_TYPE_CCT,
  "cct"
);

static const MiLightRemoteConfig FUT092Config( //rgb+cct
  new RgbCctPacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[2],
  REMOTE_TYPE_RGB_CCT,
  "rgb_cct"
);

static const MiLightRemoteConfig FUT089Config( //rgb+cct B8 / FUT089
  new FUT089PacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[2],
  REMOTE_TYPE_FUT089,
  "fut089"
);

static const MiLightRemoteConfig FUT098Config( //rgb
  new RgbPacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[3],
  REMOTE_TYPE_RGB,
  "rgb"
);

#endif
