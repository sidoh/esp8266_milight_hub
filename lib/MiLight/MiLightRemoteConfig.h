#include <MiLightRadioConfig.h>
#include <PacketFormatter.h>

#include <RgbwPacketFormatter.h>
#include <RgbPacketFormatter.h>
#include <RgbCctPacketFormatter.h>
#include <CctPacketFormatter.h>
#include <FUT089PacketFormatter.h>
#include <FUT091PacketFormatter.h>
#include <FUT020PacketFormatter.h>
#include <PacketFormatter.h>

#ifndef _MILIGHT_REMOTE_CONFIG_H
#define _MILIGHT_REMOTE_CONFIG_H

class MiLightRemoteConfig {
public:
  MiLightRemoteConfig(
    PacketFormatter* packetFormatter,
    MiLightRadioConfig& radioConfig,
    const MiLightRemoteType type,
    const String name,
    const size_t numGroups
  ) : packetFormatter(packetFormatter),
      radioConfig(radioConfig),
      type(type),
      name(name),
      numGroups(numGroups)
  { }

  PacketFormatter* const packetFormatter;
  const MiLightRadioConfig& radioConfig;
  const MiLightRemoteType type;
  const String name;
  const size_t numGroups;

  static const MiLightRemoteConfig* fromType(MiLightRemoteType type);
  static const MiLightRemoteConfig* fromType(const String& type);
  static const MiLightRemoteConfig* fromReceivedPacket(const MiLightRadioConfig& radioConfig, const uint8_t* packet, const size_t len);

  static const size_t NUM_REMOTES;
  static const MiLightRemoteConfig* ALL_REMOTES[];
};

extern const MiLightRemoteConfig FUT096Config; //rgbw
extern const MiLightRemoteConfig FUT007Config; //cct
extern const MiLightRemoteConfig FUT092Config; //rgb+cct
extern const MiLightRemoteConfig FUT089Config; //rgb+cct B8 / FUT089
extern const MiLightRemoteConfig FUT098Config; //rgb
extern const MiLightRemoteConfig FUT091Config; //v2 cct
extern const MiLightRemoteConfig FUT020Config;

#endif
