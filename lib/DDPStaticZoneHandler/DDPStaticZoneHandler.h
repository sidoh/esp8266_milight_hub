#pragma once

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <Settings.h>
#include <WiFiUdp.h>
#include <MiLightClient.h>
#include <GroupState.h>
#include <GroupStateStore.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DDP_UDP_PORT 4048
#define DDP_HEADER_SIZE 10
#define NUM_FUT089_ZONES 8

#define LD_MQTT_AVAILABILITY_TOPIC "milight/client_status"
#define LD_ECHO_SUPPRESS_MS 500
#define DDP_FAST_REPEATS 6
#define DDP_ANCHOR_REPEATS 30
#define DDP_MOTION_WINDOW_MS 180
#define DDP_ANCHOR_SETTLE_MS 150

// Below raw channel value 20 (~8% brightness), 8-bit integer truncation
// distorts R:G:B and CW:WW ratios. Freeze Hue/Sat/Kelvin below this threshold.
#define DDP_LOW_BRI_FREEZE_THRESHOLD 20

struct MilightCommand {
    bool power_on;
    bool is_white_mode;
    uint16_t hue;       // 0 - 359 degrees (passed to milightClient->updateHue)
    uint8_t saturation; // 0 - 100 (0 = pure white, 100 = pure color)
    uint8_t kelvin;     // 0 - 100 (0 = warmest WW, 100 = coldest CW)
    uint8_t brightness; // 0 - 100 (Mi-Light brightness scale)
};

class DDPStaticZoneHandler {
public:
    bool publishRawMqtt(const char* topic, const char* payload, bool retain = false);
    DDPStaticZoneHandler(MiLightClient* client, uint16_t deviceId, GroupStateStore* store, Settings& settings);
    void begin();
    void loop();
    void setDeviceId(uint16_t deviceId);
    uint16_t getDeviceId() const { return defaultDeviceId; }
    uint32_t getFastBurstsSent() const { return fastBurstsSent; }
    uint32_t getAnchorBurstsSent() const { return anchorBurstsSent; }
    void handleMqttMessage(char* topic, uint8_t* payload, unsigned int length);
    void publishLocalStateToMqtt(uint16_t deviceId, uint8_t groupId, MiLightRemoteType deviceType, const GroupState* state);
    void publishSniffedRemoteEvent(uint16_t deviceId, uint8_t groupId, const String& remoteTypeName, const JsonObject& parsedPacket);

private:
    WiFiUDP udp;
    WiFiClient wifiClient;
    PubSubClient mqtt;
    MiLightClient* milightClient;
    GroupStateStore* stateStore;
    Settings& settings;
    String mqttHostStr;
    uint16_t mqttPortNum;
    uint16_t defaultDeviceId;

    bool initializedFlags[NUM_FUT089_ZONES];
    MilightCommand activeCmds[NUM_FUT089_ZONES];
    MilightCommand pendingCmds[NUM_FUT089_ZONES];
    bool dirtyFlags[NUM_FUT089_ZONES];
    bool anchorPendingFlags[NUM_FUT089_ZONES];
    uint32_t lastZoneDirtyTs[NUM_FUT089_ZONES];
    uint32_t prevZoneDirtyGapMs[NUM_FUT089_ZONES];
    uint8_t rrZoneCursor;
    uint32_t fastBurstsSent;
    uint32_t anchorBurstsSent;

    bool isInternalTx;
    uint16_t nextMqttPacketId;
    uint8_t pendingInternalPackets[NUM_FUT089_ZONES];
    uint32_t lastInternalCommitTs[NUM_FUT089_ZONES];
    uint32_t lastInboundSyncTs[NUM_FUT089_ZONES];
    uint32_t lastOutboundPublishTs[NUM_FUT089_ZONES];
    char lastPublishedPayload[NUM_FUT089_ZONES][192];

    uint32_t lastDdpPacketTs;
    uint32_t lastTxTs;
    uint32_t lastMqttRetryTs;
    uint32_t mqttRetryIntervalMs;
    bool mqttOfflineLogged;

    void maintainMqtt();
    bool publishQoS1Retained(const char* topic, const char* payload);
    void syncHubGroupStateFromCommand(uint8_t zoneIdx, const MilightCommand& cmd);
    static void hsvToRgb(uint16_t h_deg, uint8_t s_pct, uint8_t v_pct, uint8_t& r, uint8_t& g, uint8_t& b);
    MilightCommand translateDDP5ChannelToMilight(uint8_t r, uint8_t g, uint8_t b, uint8_t cw, uint8_t ww);
    bool isCommandSignificantlyDifferent(uint8_t zoneIdx, const MilightCommand& nextCmd);
    void updateZoneFromRawBytes(uint8_t zoneIdx, const uint8_t* bytes);
    void parseDDPPacket(const uint8_t* header, const uint8_t* payload, size_t payloadLen);
    void markZoneDirty(uint8_t zoneIdx, const MilightCommand& cmd);
    void processDirtyZones();
    void commitZoneState(uint8_t zoneIndex, size_t repeatsOverride, bool forceFullSync);
};
