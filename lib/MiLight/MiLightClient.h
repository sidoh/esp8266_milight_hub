#include <functional>
#include <Arduino.h>
#include <MiLightRadio.h>
#include <MiLightRadioFactory.h>
#include <MiLightRemoteConfig.h>
#include <Settings.h>
#include <GroupStateStore.h>
#include <PacketSender.h>
#include <TransitionController.h>
#include <cstring>
#include <map>
#include <set>

#ifndef _MILIGHTCLIENT_H
#define _MILIGHTCLIENT_H

//#define DEBUG_PRINTF
//#define DEBUG_CLIENT_COMMANDS     // enable to show each individual change command (like hue, brightness, etc)

#define FS(str) (reinterpret_cast<const __FlashStringHelper*>(str))

namespace RequestKeys {
  static const char TRANSITION[] = "transition";
};

namespace TransitionParams {
  static const char FIELD[] PROGMEM = "field";
  static const char START_VALUE[] PROGMEM = "start_value";
  static const char END_VALUE[] PROGMEM = "end_value";
  static const char DURATION[] PROGMEM = "duration";
  static const char PERIOD[] PROGMEM = "period";
}

// Used to determine RGB colros that are approximately white
#define RGB_WHITE_THRESHOLD 10

class MiLightClient {
public:
  // Used to indicate that the start value for a transition should be fetched from current state
  static const int16_t FETCH_VALUE_FROM_STATE = -1;

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
  bool handleTransition(JsonObject args, JsonDocument& responseObj);
  void handleTransition(GroupStateField field, JsonVariant value, float duration, int16_t startValue = FETCH_VALUE_FROM_STATE);
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

  uint8_t parseStatus(JsonVariant object);
  JsonVariant extractStatus(JsonObject object);

protected:
  struct cmp_str {
    bool operator()(char const *a, char const *b) const {
        return std::strcmp(a, b) < 0;
    }
  };
  static const std::map<const char*, std::function<void(MiLightClient*, JsonVariant)>, cmp_str> FIELD_SETTERS;
  static const char* FIELD_ORDERINGS[];

  RadioSwitchboard& radioSwitchboard;
  std::vector<std::shared_ptr<MiLightRadio>> radios;
  std::shared_ptr<MiLightRadio> currentRadio;
  const MiLightRemoteConfig* currentRemote;

  EventHandler updateBeginHandler;
  EventHandler updateEndHandler;

  GroupStateStore* stateStore;
  const GroupState* currentState;
  Settings& settings;
  PacketSender& packetSender;
  TransitionController& transitions;

  // If set, override the number of packet repeats used.
  size_t repeatsOverride;

  void flushPacket();
};

#endif
