#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <MiLightClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DDP_UDP_PORT 4048
#define DDP_HEADER_SIZE 10
#define NUM_FUT089_ZONES 8

#define LD_MQTT_HOST "192.168.6.196"
#define LD_MQTT_PORT 1883

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
    DDPStaticZoneHandler(MiLightClient* client, uint16_t deviceId = 0x125C);
    void begin();
    void loop();
    void setDeviceId(uint16_t deviceId);
    uint16_t getDeviceId() const { return defaultDeviceId; }
    void handleMqttMessage(char* topic, uint8_t* payload, unsigned int length);

private:
    WiFiUDP udp;
    WiFiClient wifiClient;
    PubSubClient mqtt;
    MiLightClient* milightClient;
    uint16_t defaultDeviceId;

    bool initializedFlags[NUM_FUT089_ZONES];
    MilightCommand activeCmds[NUM_FUT089_ZONES];
    MilightCommand pendingCmds[NUM_FUT089_ZONES];
    bool dirtyFlags[NUM_FUT089_ZONES];

    uint32_t lastDdpPacketTs;
    uint32_t lastTxTs;
    uint32_t lastMqttRetryTs;

    void maintainMqtt();
    MilightCommand translateDDP5ChannelToMilight(uint8_t r, uint8_t g, uint8_t b, uint8_t cw, uint8_t ww);
    bool isCommandSignificantlyDifferent(uint8_t zoneIdx, const MilightCommand& nextCmd);
    void updateZoneFromRawBytes(uint8_t zoneIdx, const uint8_t* bytes);
    void parseDDPPacket(const uint8_t* header, const uint8_t* payload, size_t payloadLen);
    void processDirtyZones();
    void commitZoneState(uint8_t zoneIndex);
};
