#include <functional>
#include <Arduino.h>
#include <MiLightRadio.h>
#include <MiLightRadioFactory.h>
#include <MiLightRemoteConfig.h>
#include <Settings.h>
#include <GroupStateStore.h>

#ifndef _MILIGHTCLIENT_H
#define _MILIGHTCLIENT_H

//#define DEBUG_PRINTF
//#define DEBUG_CLIENT_COMMANDS     // enable to show each individual change command (like hue, brightness, etc)

#define MILIGHT_DEFAULT_RESEND_COUNT 10

// Used to determine RGB colros that are approximately white
#define RGB_WHITE_THRESHOLD 10

class MiLightClient {
public:
  MiLightClient(
    std::shared_ptr<MiLightRadioFactory> radioFactory,
    GroupStateStore* stateStore,
    Settings* settings
  );

  ~MiLightClient() { }

  typedef std::function<void(uint8_t* packet, const MiLightRemoteConfig& config)> PacketSentHandler;
  typedef std::function<void(void)> EventHandler;

  void begin();
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

  // CCT methods
  void updateTemperature(const uint8_t colorTemperature);
  void decreaseTemperature();
  void increaseTemperature();
  void increaseBrightness();
  void decreaseBrightness();

  void updateSaturation(const uint8_t saturation);

  void update(JsonObject object);
  void handleCommand(const String& command);
  void handleEffect(const String& effect);

  void onPacketSent(PacketSentHandler handler);
  void onUpdateBegin(EventHandler handler);
  void onUpdateEnd(EventHandler handler);

  size_t getNumRadios() const;
  std::shared_ptr<MiLightRadio> switchRadio(size_t radioIx);
  std::shared_ptr<MiLightRadio> switchRadio(const MiLightRemoteConfig* remoteConfig);
  MiLightRemoteConfig& currentRemoteConfig() const;

protected:

  std::vector<std::shared_ptr<MiLightRadio>> radios;
  std::shared_ptr<MiLightRadio> currentRadio;
  const MiLightRemoteConfig* currentRemote;

  PacketSentHandler packetSentHandler;
  EventHandler updateBeginHandler;
  EventHandler updateEndHandler;

  GroupStateStore* stateStore;
  const Settings* settings;

  // Used to track auto repeat limiting
  unsigned long lastSend;
  uint8_t currentResendCount;
  unsigned int baseResendCount;

  // This will be pre-computed, but is simply:
  //
  //    (sensitivity / 1000.0) * R
  //
  // Where R is the base number of repeats.
  size_t throttleMultiplier;

  /*
   * Calculates the number of resend packets based on when the last packet
   * was sent using this function:
   *
   *    lastRepeatsValue + (millisSinceLastSend - THRESHOLD) * throttleMultiplier
   *
   * When the last send was more recent than THRESHOLD, the number of repeats
   * will be decreased to a minimum of zero.  When less recent, it will be
   * increased up to a maximum of the default resend count.
   */
  void updateResendCount();

  uint8_t parseStatus(JsonObject object);

  void flushPacket();
};

#endif
