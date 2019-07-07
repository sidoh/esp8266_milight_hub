#include <functional>
#include <Arduino.h>
#include <MiLightRadio.h>
#include <MiLightRadioFactory.h>
#include <MiLightRemoteConfig.h>
#include <Settings.h>
#include <GroupStateStore.h>
#include <PacketSender.h>
#include <TransitionController.h>

#ifndef _MILIGHTCLIENT_H
#define _MILIGHTCLIENT_H

//#define DEBUG_PRINTF
//#define DEBUG_CLIENT_COMMANDS     // enable to show each individual change command (like hue, brightness, etc)

// Used to determine RGB colros that are approximately white
#define RGB_WHITE_THRESHOLD 10

class MiLightClient {
public:
  MiLightClient(
    RadioSwitchboard& radioSwitchboard,
    PacketSender& packetSender,
    GroupStateStore* stateStore,
    Settings& settings,
    TransitionController& transitions
  );

  ~MiLightClient() { }

  typedef std::function<void(void)> EventHandler;

  void prepare(const MiLightRemoteConfig* remoteConfig, const uint16_t deviceId = -1, const uint8_t groupId = -1);
  void prepare(const MiLightRemoteType type, const uint16_t deviceId = -1, const uint8_t groupId = -1);

  void setResendCount(const unsigned int resendCount);
  bool available();
  size_t read(uint8_t packet[]);
  void write(uint8_t packet[]);

  void setHeld(bool held);

  // Common methods
  void updateStatus(MiLightStatus status);
  void updateStatus(MiLightStatus status, uint8_t groupId);
  void pair();
  void unpair();
  void command(uint8_t command, uint8_t arg);
  void updateMode(uint8_t mode);
  void nextMode();
  void previousMode();
  void modeSpeedDown();
  void modeSpeedUp();
  void toggleStatus();

  // RGBW methods
  void updateHue(const uint16_t hue);
  void updateBrightness(const uint8_t brightness);
  void updateColorWhite();
  void updateColorRaw(const uint8_t color);
  void enableNightMode();
  void updateColor(JsonVariant json);

  // CCT methods
  void updateTemperature(const uint8_t colorTemperature);
  void decreaseTemperature();
  void increaseTemperature();
  void increaseBrightness();
  void decreaseBrightness();

  void updateSaturation(const uint8_t saturation);

  void update(JsonObject object);
  void handleCommand(JsonVariant command);
  void handleCommands(JsonArray commands);
  void handleTransition(JsonObject args);
  void handleEffect(const String& effect);

  void onUpdateBegin(EventHandler handler);
  void onUpdateEnd(EventHandler handler);

  size_t getNumRadios() const;
  std::shared_ptr<MiLightRadio> switchRadio(size_t radioIx);
  std::shared_ptr<MiLightRadio> switchRadio(const MiLightRemoteConfig* remoteConfig);
  MiLightRemoteConfig& currentRemoteConfig() const;

  // Call to override the number of packet repeats that are sent.  Clear with clearRepeatsOverride
  void setRepeatsOverride(size_t repeatsOverride);

  // Clear the repeats override so that the default is used
  void clearRepeatsOverride();

  uint8_t parseStatus(JsonObject object);

protected:
  static const std::map<const char*, std::function<void(MiLightClient*, JsonVariant)>> FIELD_SETTERS;

  RadioSwitchboard& radioSwitchboard;
  std::vector<std::shared_ptr<MiLightRadio>> radios;
  std::shared_ptr<MiLightRadio> currentRadio;
  const MiLightRemoteConfig* currentRemote;

  EventHandler updateBeginHandler;
  EventHandler updateEndHandler;

  GroupStateStore* stateStore;
  Settings& settings;
  PacketSender& packetSender;
  TransitionController& transitions;

  // If set, override the number of packet repeats used.
  size_t repeatsOverride;

  void flushPacket();
};

#endif
