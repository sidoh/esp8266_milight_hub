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

#define MILIGHT_DEFAULT_RESEND_COUNT 10
//Used to determine close to white
#define RGB_WHITE_BOUNDARY 40

class MiLightClient {
public:
  MiLightClient(
    MiLightRadioFactory* radioFactory,
    GroupStateStore& stateStore,
    size_t throttleThreshold,
    size_t throttleSensitivity,
    size_t packetRepeatMinimum
  );

  ~MiLightClient() {
    delete[] radios;
  }

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

  void update(const JsonObject& object);
  void handleCommand(const String& command);
  void handleEffect(const String& effect);

  void onPacketSent(PacketSentHandler handler);
  void onUpdateBegin(EventHandler handler);
  void onUpdateEnd(EventHandler handler);

  size_t getNumRadios() const;
  MiLightRadio* switchRadio(size_t radioIx);
  MiLightRemoteConfig& currentRemoteConfig() const;

protected:

  MiLightRadio** radios;
  MiLightRadio* currentRadio;
  const MiLightRemoteConfig* currentRemote;
  const size_t numRadios;
  GroupStateStore& stateStore;

  PacketSentHandler packetSentHandler;
  EventHandler updateBeginHandler;
  EventHandler updateEndHandler;

  // Used to track auto repeat limiting
  unsigned long lastSend;
  int currentResendCount;
  unsigned int baseResendCount;
  int packetRepeatMinimum;
  size_t throttleThreshold;
  size_t throttleSensitivity;

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

  MiLightRadio* switchRadio(const MiLightRemoteConfig* remoteConfig);
  uint8_t parseStatus(const JsonObject& object);

  void flushPacket();
};

#endif
