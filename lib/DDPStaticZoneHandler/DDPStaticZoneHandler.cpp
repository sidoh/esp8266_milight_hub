#include "DDPStaticZoneHandler.h"
#include <math.h>

static DDPStaticZoneHandler* s_ddpInstance = nullptr;

// Circular difference on 0..359 degree wheel
static inline int hueCircularDiff360(uint16_t a, uint16_t b) {
    int diff = abs((int)a - (int)b);
    return (diff > 180) ? (360 - diff) : diff;
}

DDPStaticZoneHandler::DDPStaticZoneHandler(MiLightClient* client, uint16_t deviceId)
    : mqtt(wifiClient), milightClient(client), defaultDeviceId(deviceId),
      lastDdpPacketTs(0), lastTxTs(0), lastMqttRetryTs(0) {
    s_ddpInstance = this;
    for (int i = 0; i < NUM_FUT089_ZONES; i++) {
        initializedFlags[i] = false;
        activeCmds[i]       = {false, true, 0, 100, 50, 0};
        pendingCmds[i]      = {false, true, 0, 100, 50, 0};
        dirtyFlags[i]       = false;
    }
}

void DDPStaticZoneHandler::setDeviceId(uint16_t deviceId) {
    defaultDeviceId = deviceId;
}

void DDPStaticZoneHandler::begin() {
    udp.stop();
    delay(20);
    udp.begin(DDP_UDP_PORT);
    Serial.printf("[DDP-HUB] Listening on UDP %d (Target ID: 0x%04X)\n", DDP_UDP_PORT, defaultDeviceId);

    mqtt.setServer(LD_MQTT_HOST, LD_MQTT_PORT);
    mqtt.setSocketTimeout(2);
    mqtt.setCallback([](char* topic, uint8_t* payload, unsigned int length) {
        if (s_ddpInstance) {
            s_ddpInstance->handleMqttMessage(topic, payload, length);
        }
    });
}

void DDPStaticZoneHandler::maintainMqtt() {
    if (WiFi.status() != WL_CONNECTED) return;

    if (mqtt.connected()) {
        mqtt.loop();
        return;
    }

    uint32_t now = millis();
    if (lastMqttRetryTs != 0 && (now - lastMqttRetryTs < 10000)) return;
    lastMqttRetryTs = now;

    String clientId = "milight-ddp-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.printf("[DDP-MQTT] Connecting to %s:%d as %s...\n", LD_MQTT_HOST, LD_MQTT_PORT, clientId.c_str());

    if (mqtt.connect(clientId.c_str())) {
        Serial.println("[DDP-MQTT] Connected to broker!");
        mqtt.subscribe("lightdancer/static/+/state", 1);
        Serial.println("[DDP-MQTT] Subscribed to: lightdancer/static/+/state");
    } else {
        Serial.printf("[DDP-MQTT] Connection failed (rc=%d), retrying in 10s...\n", mqtt.state());
    }
}

MilightCommand DDPStaticZoneHandler::translateDDP5ChannelToMilight(uint8_t r, uint8_t g, uint8_t b, uint8_t cw, uint8_t ww) {
    MilightCommand cmd = {false, true, 0, 100, 50, 0};

    uint8_t rgb_max = max(r, max(g, b));
    uint8_t rgb_min = min(r, min(g, b));
    uint16_t white_sum = (uint16_t)cw + (uint16_t)ww;

    // 1. BLACKOUT / OFF DETECTION (Noise floor <= 2)
    if (rgb_max <= 2 && white_sum <= 2) {
        cmd.power_on = false;
        return cmd;
    }
    cmd.power_on = true;

    // 2. MODE ARBITRATION: WHITE (CCT) MODE vs. COLOR (RGB) MODE
    uint8_t rgb_chroma = rgb_max - rgb_min;

    if (rgb_chroma < 8 && white_sum > 0) {
        // ---> PURE WHITE (CCT) MODE <---
        cmd.is_white_mode = true;
        cmd.kelvin = (uint8_t)round(((float)cw / (float)white_sum) * 100.0f);

        uint16_t effective_white = min((uint16_t)255, (uint16_t)(white_sum + rgb_min));
        cmd.brightness = (uint8_t)round(((float)effective_white / 255.0f) * 100.0f);
        cmd.brightness = max((uint8_t)1, cmd.brightness);
        return cmd;
    }

    // ---> COLOR (RGB + SATURATION) MODE <---
    cmd.is_white_mode = false;

    // 3. EXTRACT HUE (0 - 359 degrees for native GroupStateField::HUE / updateHue)
    float h_deg = 0.0f;
    if (rgb_chroma > 0) {
        if (rgb_max == r) {
            h_deg = 60.0f * fmod(((float)(g - b) / (float)rgb_chroma), 6.0f);
        } else if (rgb_max == g) {
            h_deg = 60.0f * (((float)(b - r) / (float)rgb_chroma) + 2.0f);
        } else {
            h_deg = 60.0f * (((float)(r - g) / (float)rgb_chroma) + 4.0f);
        }
        if (h_deg < 0.0f) h_deg += 360.0f;
    }
    cmd.hue = ((uint16_t)round(h_deg)) % 360;

    // 4. EXTRACT UNIFIED SATURATION (Combining RGB Desaturation + CW/WW White Mix)
    float total_whiteness = (float)rgb_min + ((float)white_sum * 0.85f);
    float total_energy = (float)rgb_chroma + total_whiteness;

    if (total_energy > 0.0f) {
        float sat_ratio = (float)rgb_chroma / total_energy;
        cmd.saturation = (uint8_t)round(constrain(sat_ratio * 100.0f, 0.0f, 100.0f));
    } else {
        cmd.saturation = 100;
    }

    // Snap cleanly into CCT White Mode if saturation drops below 5%
    if (cmd.saturation < 5) {
        cmd.is_white_mode = true;
        cmd.kelvin = (white_sum > 0) ? (uint8_t)round(((float)cw / (float)white_sum) * 100.0f) : 50;
        uint8_t peak = max(rgb_max, (uint8_t)min((uint16_t)255, white_sum));
        cmd.brightness = max((uint8_t)1, (uint8_t)round(((float)peak / 255.0f) * 100.0f));
        return cmd;
    }

    // 5. EXTRACT MASTER BRIGHTNESS (0 - 100%)
    uint16_t combined_peak = min((uint16_t)255, (uint16_t)max((int)rgb_max, (int)white_sum));
    cmd.brightness = max((uint8_t)1, (uint8_t)round(((float)combined_peak / 255.0f) * 100.0f));

    return cmd;
}

bool DDPStaticZoneHandler::isCommandSignificantlyDifferent(uint8_t zoneIdx, const MilightCommand& nextCmd) {
    if (!initializedFlags[zoneIdx]) return true;

    const MilightCommand& cur = activeCmds[zoneIdx];
    if (nextCmd.power_on != cur.power_on) return true;
    if (!nextCmd.power_on) return false; // Both are OFF

    if (nextCmd.is_white_mode != cur.is_white_mode) return true;
    if (abs((int)nextCmd.brightness - (int)cur.brightness) >= 2) return true;

    if (nextCmd.is_white_mode) {
        if (abs((int)nextCmd.kelvin - (int)cur.kelvin) >= 2) return true;
    } else {
        if (hueCircularDiff360(nextCmd.hue, cur.hue) >= 2) return true;
        if (abs((int)nextCmd.saturation - (int)cur.saturation) >= 2) return true;
    }
    return false;
}

void DDPStaticZoneHandler::handleMqttMessage(char* topic, uint8_t* payload, unsigned int length) {
    if (!topic || !payload || length == 0 || length > 255) return;

    const char* prefix = "lightdancer/static/s_zone";
    const char* sub = strstr(topic, prefix);
    if (!sub) return;

    int zoneNum = sub[strlen(prefix)] - '0';
    if (zoneNum < 1 || zoneNum > NUM_FUT089_ZONES) return;
    int zoneIdx = zoneNum - 1;

    char safeBuf[256];
    memcpy(safeBuf, payload, length);
    safeBuf[length] = '\0';

    StaticJsonDocument<384> doc;
    if (deserializeJson(doc, safeBuf)) return;

    bool on     = doc["on"] | false;
    uint8_t bri = doc["bri"] | 255;

    // EDGE CASE 1A: Extract un-multiplied raw 0-255 channels directly from MQTT JSON
    uint8_t raw_r  = doc["r"]  | 0;
    uint8_t raw_g  = doc["g"]  | 0;
    uint8_t raw_b  = doc["b"]  | 0;
    uint8_t raw_cw = doc["cw"] | 0;
    uint8_t raw_ww = doc["ww"] | 0;

    MilightCommand cmd;
    if (!on || bri == 0) {
        cmd = activeCmds[zoneIdx];
        cmd.power_on   = false;
        cmd.brightness = 0;
    } else {
        cmd = translateDDP5ChannelToMilight(raw_r, raw_g, raw_b, raw_cw, raw_ww);
        if (cmd.power_on) {
            float channel_peak = (float)min((uint16_t)255, (uint16_t)max((int)max(raw_r, max(raw_g, raw_b)), (int)((uint16_t)raw_cw + (uint16_t)raw_ww))) / 255.0f;
            float effective_bri = ((float)bri / 255.0f) * channel_peak * 100.0f;
            cmd.brightness = max((uint8_t)1, (uint8_t)round(effective_bri));
        }
    }

    if (isCommandSignificantlyDifferent(zoneIdx, cmd)) {
        pendingCmds[zoneIdx] = cmd;
        dirtyFlags[zoneIdx]  = true;
        Serial.printf("[DDP-MQTT] Zone %d Unscaled -> ON:%d Mode:%s Bri:%d%% Hue:%d° Sat:%d%% Kel:%d%%\n",
                      zoneNum, cmd.power_on, cmd.is_white_mode ? "WHITE" : "COLOR",
                      cmd.brightness, cmd.hue, cmd.saturation, cmd.kelvin);
    }
}

void DDPStaticZoneHandler::loop() {
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
        uint8_t rxBuffer[512];
        int len = udp.read(rxBuffer, sizeof(rxBuffer));

        if (len >= DDP_HEADER_SIZE && ((rxBuffer[0] & 0xC0) == 0x40)) {
            lastDdpPacketTs = millis();
            parseDDPPacket(rxBuffer, rxBuffer + DDP_HEADER_SIZE, len - DDP_HEADER_SIZE);
        }
    }
    maintainMqtt();
    processDirtyZones();
}

void DDPStaticZoneHandler::updateZoneFromRawBytes(uint8_t zoneIdx, const uint8_t* bytes) {
    if (zoneIdx >= NUM_FUT089_ZONES) return;

    uint8_t r  = bytes[0];
    uint8_t g  = bytes[1];
    uint8_t b  = bytes[2];
    uint8_t cw = bytes[3];
    uint8_t ww = bytes[4];

    uint8_t rgb_max = max(r, max(g, b));

    MilightCommand cmd = translateDDP5ChannelToMilight(r, g, b, cw, ww);

    // EDGE CASE 1B: Low-End Hue Lock when max(R,G,B) < 20 on raw DDP frames
    if (cmd.power_on && rgb_max < DDP_LOW_BRI_FREEZE_THRESHOLD && initializedFlags[zoneIdx]) {
        const MilightCommand& locked = pendingCmds[zoneIdx].power_on ? pendingCmds[zoneIdx] : activeCmds[zoneIdx];
        if (locked.power_on) {
            cmd.hue = locked.hue;
            if (!locked.is_white_mode && (uint16_t)cw + (uint16_t)ww == 0) {
                cmd.is_white_mode = false;
                cmd.saturation    = locked.saturation;
            }
        }
    }

    if (isCommandSignificantlyDifferent(zoneIdx, cmd)) {
        pendingCmds[zoneIdx] = cmd;
        dirtyFlags[zoneIdx]  = true;
    }
}

void DDPStaticZoneHandler::parseDDPPacket(const uint8_t* header, const uint8_t* payload, size_t actualPayloadLen) {
    // EDGE CASE 2: Check DDP Header Payload Length (header[8] << 8 | header[9])
    uint16_t headerDataLen = ((uint16_t)header[8] << 8) | (uint16_t)header[9];
    size_t effectiveLen = (headerDataLen > 0 && headerDataLen <= actualPayloadLen)
                          ? (size_t)headerDataLen
                          : actualPayloadLen;

    const size_t stride = 5;

    if (effectiveLen == 40) {
        // 8-zone multiplexed frame (Bytes 0..4 = Zone 1 ... Bytes 35..39 = Zone 8)
        for (uint8_t i = 0; i < NUM_FUT089_ZONES; i++) {
            updateZoneFromRawBytes(i, payload + (i * stride));
        }
    } else if (effectiveLen == 5) {
        // Single-zone frame (check DDP 32-bit channel offset in header[4..7], default to Zone 1)
        uint32_t byteOffset = ((uint32_t)header[4] << 24) |
                              ((uint32_t)header[5] << 16) |
                              ((uint32_t)header[6] << 8)  |
                              ((uint32_t)header[7]);
        uint8_t targetZoneIdx = (byteOffset / stride) < NUM_FUT089_ZONES ? (byteOffset / stride) : 0;
        updateZoneFromRawBytes(targetZoneIdx, payload);
    } else {
        // Fallback for general N-zone multiples of 5 bytes
        uint8_t count = min((size_t)NUM_FUT089_ZONES, effectiveLen / stride);
        for (uint8_t i = 0; i < count; i++) {
            updateZoneFromRawBytes(i, payload + (i * stride));
        }
    }
}

void DDPStaticZoneHandler::processDirtyZones() {
    uint32_t now = millis();
    if (now - lastTxTs < 65) return;

    for (uint8_t i = 0; i < NUM_FUT089_ZONES; i++) {
        if (dirtyFlags[i]) {
            commitZoneState(i);
            activeCmds[i]       = pendingCmds[i];
            initializedFlags[i] = true;
            dirtyFlags[i]       = false;
            lastTxTs            = now;
            break;
        }
    }
}

void DDPStaticZoneHandler::commitZoneState(uint8_t zoneIndex) {
    uint8_t zoneId = zoneIndex + 1;
    const MilightCommand& target = pendingCmds[zoneIndex];
    const MilightCommand& prev   = activeCmds[zoneIndex];
    bool firstRun                = !initializedFlags[zoneIndex];

    milightClient->prepare(MiLightRemoteType::REMOTE_TYPE_FUT089, defaultDeviceId, zoneId);

    // 1. Handle OFF
    if (!target.power_on) {
        if (firstRun || prev.power_on) {
            milightClient->updateStatus(MiLightStatus::OFF);
            Serial.printf("[DDP-HUB] Zone %d -> OFF\n", zoneId);
        }
        return;
    }

    // 2. Wake bulb if transitioning from OFF -> ON
    bool justTurnedOn = firstRun || !prev.power_on;
    bool modeChanged  = firstRun || justTurnedOn || (target.is_white_mode != prev.is_white_mode);

    if (justTurnedOn) {
        milightClient->updateStatus(MiLightStatus::ON);
    }

    // 3. Delta-filtered White (CCT) Mode transmission
    if (target.is_white_mode) {
        bool kelvinChanged = modeChanged || (abs((int)target.kelvin - (int)prev.kelvin) >= 2);
        bool briChanged    = modeChanged || (abs((int)target.brightness - (int)prev.brightness) >= 2);

        if (modeChanged && !kelvinChanged) {
            milightClient->updateColorWhite();
        }
        if (kelvinChanged) {
            milightClient->updateTemperature(target.kelvin);
        }
        if (briChanged) {
            milightClient->updateBrightness(target.brightness);
        }

        Serial.printf("[DDP-HUB] Zone %d -> WHITE [Kelvin:%d%%, Bri:%d%%] (Delta: Mode:%d Kel:%d Bri:%d)\n",
                      zoneId, target.kelvin, target.brightness, modeChanged, kelvinChanged, briChanged);
        return;
    }

    // 4. Delta-filtered Color (RGB + Saturation) Mode transmission
    // EDGE CASE 3: Use milightClient->updateHue(0..359) so the hub's internal
    // ParsedColor / FUT089 formatter applies the exact hardware color wheel offset!
    bool hueChanged = modeChanged || (hueCircularDiff360(target.hue, prev.hue) >= 2);
    bool satChanged = modeChanged || (abs((int)target.saturation - (int)prev.saturation) >= 2);
    bool briChanged = modeChanged || (abs((int)target.brightness - (int)prev.brightness) >= 2);

    if (hueChanged) {
        milightClient->updateHue(target.hue);
    }
    if (satChanged) {
        milightClient->updateSaturation(target.saturation);
    }
    if (briChanged) {
        milightClient->updateBrightness(target.brightness);
    }

    Serial.printf("[DDP-HUB] Zone %d -> COLOR [Hue:%d°, Sat:%d%%, Bri:%d%%] (Delta: Mode:%d Hue:%d Sat:%d Bri:%d)\n",
                  zoneId, target.hue, target.saturation, target.brightness,
                  modeChanged, hueChanged, satChanged, briChanged);
}
