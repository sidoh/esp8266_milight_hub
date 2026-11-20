#include "DDPStaticZoneHandler.h"
#include <math.h>

static DDPStaticZoneHandler* s_ddpInstance = nullptr;

// Circular difference on 0..359 degree wheel
static inline int hueCircularDiff360(uint16_t a, uint16_t b) {
    int diff = abs((int)a - (int)b);
    return (diff > 180) ? (360 - diff) : diff;
}

DDPStaticZoneHandler::DDPStaticZoneHandler(MiLightClient* client, uint16_t deviceId, GroupStateStore* store)
    : mqtt(wifiClient), milightClient(client), stateStore(store), defaultDeviceId(deviceId),
      isInternalTx(false), nextMqttPacketId(1),
      lastDdpPacketTs(0), lastTxTs(0), lastMqttRetryTs(0),
      mqttRetryIntervalMs(10000), mqttOfflineLogged(false),
      rrZoneCursor(0), fastBurstsSent(0), anchorBurstsSent(0) {
    s_ddpInstance = this;
    for (int i = 0; i < NUM_FUT089_ZONES; i++) {
        initializedFlags[i]        = false;
        activeCmds[i]              = {false, true, 0, 100, 50, 0};
        pendingCmds[i]             = {false, true, 0, 100, 50, 0};
        dirtyFlags[i]              = false;
        anchorPendingFlags[i]      = false;
        lastZoneDirtyTs[i]         = 0;
        prevZoneDirtyGapMs[i]      = 9999;
        pendingInternalPackets[i]  = 0;
        lastInternalCommitTs[i]    = 0;
        lastInboundSyncTs[i]       = 0;
        lastOutboundPublishTs[i]   = 0;
        lastPublishedPayload[i][0] = '\0';
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
    mqtt.setBufferSize(512);
    mqtt.setSocketTimeout(2);
    mqtt.setCallback([](char* topic, uint8_t* payload, unsigned int length) {
        if (s_ddpInstance) {
            s_ddpInstance->handleMqttMessage(topic, payload, length);
        }
    });
}

// Raw MQTT v3.1.1 PUBLISH packet sender with QoS = 1 and Retain = true
// (PubSubClient::publish only supports QoS 0; broker PUBACK is safely ignored by PubSubClient::loop)
bool DDPStaticZoneHandler::publishQoS1Retained(const char* topic, const char* payload) {
    if (!mqtt.connected() || !wifiClient.connected()) return false;

    size_t topicLen = strlen(topic);
    size_t payloadLen = strlen(payload);
    size_t remainingLen = 2 + topicLen + 2 + payloadLen; // Topic len (2) + Topic + PacketId (2) + Payload
    if (remainingLen > 380) return false;

    uint8_t frame[400];
    size_t pos = 0;

    // Fixed Header: PUBLISH (0x30) | QoS 1 (0x02) | RETAIN (0x01) = 0x33
    frame[pos++] = 0x33;

    // Variable-length Remaining Length encoding
    size_t rem = remainingLen;
    do {
        uint8_t encodedByte = rem % 128;
        rem /= 128;
        if (rem > 0) encodedByte |= 0x80;
        frame[pos++] = encodedByte;
    } while (rem > 0);

    // Topic Length (MSB, LSB) + Topic String
    frame[pos++] = (uint8_t)((topicLen >> 8) & 0xFF);
    frame[pos++] = (uint8_t)(topicLen & 0xFF);
    memcpy(frame + pos, topic, topicLen);
    pos += topicLen;

    // Packet Identifier (non-zero 16-bit ID required for QoS 1)
    if (nextMqttPacketId == 0) nextMqttPacketId = 1;
    uint16_t pid = nextMqttPacketId++;
    frame[pos++] = (uint8_t)((pid >> 8) & 0xFF);
    frame[pos++] = (uint8_t)(pid & 0xFF);

    // Payload bytes
    memcpy(frame + pos, payload, payloadLen);
    pos += payloadLen;

    size_t written = wifiClient.write(frame, pos);
    return (written == pos);
}

void DDPStaticZoneHandler::maintainMqtt() {
    if (WiFi.status() != WL_CONNECTED) return;

    if (mqtt.connected()) {
        mqtt.loop();
        return;
    }

    uint32_t now = millis();
    if (lastMqttRetryTs != 0 && (now - lastMqttRetryTs < mqttRetryIntervalMs)) return;
    lastMqttRetryTs = now;

    // Fast 150ms LAN TCP pre-check so an offline/rebooting Orange Pi never stalls UDP or RF!
    wifiClient.stop();
    if (!wifiClient.connect(LD_MQTT_HOST, LD_MQTT_PORT, 150)) {
        wifiClient.stop();
        if (!mqttOfflineLogged) {
            Serial.printf("[DDP-MQTT] Broker %s:%d unreachable; backing off silently (10s..60s) until online.\n",
                          LD_MQTT_HOST, LD_MQTT_PORT);
            mqttOfflineLogged = true;
        }
        mqttRetryIntervalMs = min((uint32_t)60000, mqttRetryIntervalMs * 2);
        return;
    }
    wifiClient.stop();

    String clientId = "milight-ddp-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    // Connect with LWT on milight/client_status -> "disconnected" (QoS 1, Retain = true)
    if (mqtt.connect(clientId.c_str(), nullptr, nullptr, LD_MQTT_AVAILABILITY_TOPIC, 1, true, "disconnected")) {
        mqttRetryIntervalMs = 10000;
        mqttOfflineLogged   = false;
        publishQoS1Retained(LD_MQTT_AVAILABILITY_TOPIC, "connected");
        mqtt.subscribe("lightdancer/static/+/state", 1);
        Serial.println("[DDP-MQTT] Connected! Availability 'connected' & subscribed (QoS 1).");
    } else {
        if (!mqttOfflineLogged) {
            Serial.printf("[DDP-MQTT] MQTT handshake failed (rc=%d); backing off silently.\n", mqtt.state());
            mqttOfflineLogged = true;
        }
        mqttRetryIntervalMs = min((uint32_t)60000, mqttRetryIntervalMs * 2);
    }
}

void DDPStaticZoneHandler::hsvToRgb(uint16_t h_deg, uint8_t s_pct, uint8_t v_pct, uint8_t& r, uint8_t& g, uint8_t& b) {
    float h = (float)(h_deg % 360);
    float s = constrain(s_pct / 100.0f, 0.0f, 1.0f);
    float v = constrain(v_pct / 100.0f, 0.0f, 1.0f);

    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r1 = 0.0f, g1 = 0.0f, b1 = 0.0f;
    if (h < 60.0f)       { r1 = c; g1 = x; b1 = 0; }
    else if (h < 120.0f) { r1 = x; g1 = c; b1 = 0; }
    else if (h < 180.0f) { r1 = 0; g1 = c; b1 = x; }
    else if (h < 240.0f) { r1 = 0; g1 = x; b1 = c; }
    else if (h < 300.0f) { r1 = x; g1 = 0; b1 = c; }
    else                 { r1 = c; g1 = 0; b1 = x; }

    r = (uint8_t)roundf((r1 + m) * 255.0f);
    g = (uint8_t)roundf((g1 + m) * 255.0f);
    b = (uint8_t)roundf((b1 + m) * 255.0f);
}

void DDPStaticZoneHandler::syncHubGroupStateFromCommand(uint8_t zoneIdx, const MilightCommand& cmd) {
    if (!stateStore) return;
    BulbId bulbId(defaultDeviceId, zoneIdx + 1, REMOTE_TYPE_FUT089);
    GroupState* gs = stateStore->get(bulbId);
    if (!gs) return;

    gs->setState(cmd.power_on ? MiLightStatus::ON : MiLightStatus::OFF);
    gs->setBrightness(cmd.brightness);
    if (cmd.is_white_mode) {
        gs->setBulbMode(BULB_MODE_WHITE);
        gs->setKelvin(cmd.kelvin);
    } else {
        gs->setBulbMode(BULB_MODE_COLOR);
        gs->setHue(cmd.hue);
        gs->setSaturation(cmd.saturation);
    }
    stateStore->set(bulbId, *gs);
}

void DDPStaticZoneHandler::publishSniffedRemoteEvent(uint16_t deviceId, uint8_t groupId, const String& remoteTypeName, const JsonObject& parsedPacket) {
    if (!mqtt.connected()) return;

    StaticJsonDocument<256> doc;
    doc["device_id"] = deviceId;
    char hexId[10];
    snprintf(hexId, sizeof(hexId), "0x%04X", deviceId);
    doc["hex_id"] = hexId;
    doc["group_id"] = groupId;
    doc["device_type"] = remoteTypeName;
    for (JsonPair kv : parsedPacket) {
        doc[kv.key()] = kv.value();
    }

    char topic[80];
    snprintf(topic, sizeof(topic), "milight/remotes/0x%04X/%s/%u", deviceId, remoteTypeName.c_str(), groupId);

    char payload[240];
    serializeJson(doc, payload, sizeof(payload));
    mqtt.publish(topic, payload, false);
}

void DDPStaticZoneHandler::publishLocalStateToMqtt(uint16_t deviceId, uint8_t groupId, MiLightRemoteType deviceType, const GroupState* state) {
    // 4. ECHO-LOOP PREVENTION GUARD (CRITICAL)
    // Internal DDP/MQTT packets are strictly filtered via QueuedPacket::isInternal before reaching here!
    if (isInternalTx || state == nullptr) return;
    bool supportedType = (deviceType == REMOTE_TYPE_FUT089 ||
                          deviceType == REMOTE_TYPE_RGB_CCT ||
                          deviceType == REMOTE_TYPE_RGBW ||
                          deviceType == REMOTE_TYPE_CCT ||
                          deviceType == REMOTE_TYPE_FUT091);
    if (deviceId != defaultDeviceId || !supportedType) return;

    // Fence Lights is natively FUT089 Zone 7

    // Expand Master Group 0 ("All Zones" on physical FUT089 remote) across Zones 1..8!
    if (groupId == 0) {
        for (uint8_t z = 1; z <= NUM_FUT089_ZONES; z++) {
            BulbId zId(defaultDeviceId, z, REMOTE_TYPE_FUT089);
            GroupState* zState = stateStore ? stateStore->get(zId) : nullptr;
            if (zState != nullptr) {
                if (state->isSetState())      zState->setState(state->getState());
                if (state->isSetBrightness()) zState->setBrightness(state->getBrightness());
                if (state->isSetBulbMode())   zState->setBulbMode(state->getBulbMode());
                if (state->isSetHue())        zState->setHue(state->getHue());
                if (state->isSetSaturation()) zState->setSaturation(state->getSaturation());
                if (state->isSetKelvin())     zState->setKelvin(state->getKelvin());
                stateStore->set(zId, *zState);
                publishLocalStateToMqtt(deviceId, z, deviceType, zState);
            } else {
                publishLocalStateToMqtt(deviceId, z, deviceType, state);
            }
        }
        return;
    }

    if (groupId < 1 || groupId > NUM_FUT089_ZONES) return;

    uint8_t zoneIdx = groupId - 1;
    uint32_t now = millis();

    // Update internal command tracker from the Hub's updated GroupState
    MilightCommand& cur = activeCmds[zoneIdx];
    if (state->isSetState()) {
        cur.power_on = (state->getState() == MiLightStatus::ON);
    } else if (state->isSetBrightness() || state->isSetHue() || state->isSetKelvin() || state->isSetSaturation()) {
        // Physical remote slider adjustment implicitly wakes the bulb if no OFF state was sent
        cur.power_on = true;
    }
    if (state->isSetBrightness()) {
        // GroupState::getBrightness() is 0..100%
        cur.brightness = constrain((int)state->getBrightness(), 10, 100);
    }
    if (cur.power_on && cur.brightness == 0) {
        // Prevent transient {"on":true,"bri":0} if an ON packet arrives before a Brightness packet
        cur.brightness = 100;
    }
    if (state->isSetBulbMode()) {
        cur.is_white_mode = (state->getBulbMode() == BULB_MODE_WHITE);
    }
    if (state->isSetHue()) {
        cur.hue = state->getHue() % 360;
    }
    if (state->isSetSaturation()) {
        cur.saturation = constrain(state->getSaturation(), 0, 100);
    }
    if (state->isSetKelvin()) {
        cur.kelvin = constrain(state->getKelvin(), 0, 100);
    }

    pendingCmds[zoneIdx]      = cur;
    initializedFlags[zoneIdx] = true;

    if (!mqtt.connected()) return;

    // Map Mi-Light GroupState (0..100%) -> Light-Dancer 5-Channel JSON Schema (0..255)
    if (cur.power_on && cur.brightness < 10) cur.brightness = 10;
    // Zone 7 is a 4-channel RGBW controller on FUT089: prevent desaturated pink/red+white states in MQTT!
    if (zoneIdx == 6 && !cur.is_white_mode && cur.saturation < 25) {
        cur.is_white_mode = true;
        cur.kelvin = 50;
    } else if (zoneIdx == 6 && !cur.is_white_mode) {
        cur.saturation = 100;
    }
    uint8_t bri255 = cur.power_on
                     ? max((uint8_t)26, (uint8_t)roundf((cur.brightness / 100.0f) * 255.0f))
                     : 0;
    uint8_t r = 0, g = 0, b = 0, cw = 0, ww = 0;

    bool isColorTempMode = state->isSetBulbMode()
                           ? (state->getBulbMode() == BULB_MODE_WHITE)
                           : cur.is_white_mode;

    if (isColorTempMode) {
        // White / CCT Mode: 153 mireds (coldest) <-> 370 mireds (warmest)
        uint16_t mireds = state->isSetKelvin() ? state->getMireds() : (uint16_t)roundf(153.0f + (cur.kelvin / 100.0f) * (370.0f - 153.0f));
        float t = constrain((370.0f - (float)mireds) / (370.0f - 153.0f), 0.0f, 1.0f);
        r  = 0;
        g  = 0;
        b  = 0;
        cw = (uint8_t)roundf(255.0f * t);
        ww = (uint8_t)roundf(255.0f * (1.0f - t));
    } else {
        // Color Mode: (hue, saturation, V=100%) -> pure un-dimmed r, g, b; cw/ww = 0
        hsvToRgb(cur.hue, cur.saturation, 100, r, g, b);
        cw = 0;
        ww = 0;
    }

    StaticJsonDocument<256> doc;
    doc["on"]  = cur.power_on;
    doc["bri"] = bri255;
    doc["r"]   = r;
    doc["g"]   = g;
    doc["b"]   = b;
    doc["cw"]  = cw;
    doc["ww"]  = ww;

    char topic[64];
    snprintf(topic, sizeof(topic), "lightdancer/static/s_zone%u/state", groupId);

    char payload[192];
    serializeJson(doc, payload, sizeof(payload));

    // Skip byte-identical duplicate UDP retries within 400ms, while ALWAYS publishing real changes immediately!
    if (strcmp(lastPublishedPayload[zoneIdx], payload) == 0 && (now - lastOutboundPublishTs[zoneIdx] < 400)) {
        return;
    }
    strncpy(lastPublishedPayload[zoneIdx], payload, sizeof(lastPublishedPayload[zoneIdx]) - 1);
    lastPublishedPayload[zoneIdx][sizeof(lastPublishedPayload[zoneIdx]) - 1] = '\0';
    lastOutboundPublishTs[zoneIdx] = now;
    publishQoS1Retained(topic, payload);

    Serial.printf("[DDP-MQTT] Outbound Local Sync (QoS1, Retain) -> %s : %s\n", topic, payload);
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
        // Note: In esp8266_milight_hub Units.h, whiteVal 0 = 153 mireds (Coldest CW)
        // and whiteVal 100 = 370 mireds (Warmest WW). Use ww / white_sum!
        cmd.is_white_mode = true;
        cmd.kelvin = (uint8_t)round(((float)ww / (float)white_sum) * 100.0f);

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
        cmd.kelvin = (white_sum > 0) ? (uint8_t)round(((float)ww / (float)white_sum) * 100.0f) : 50;
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

    uint32_t now = millis();
    // Ignore immediate broker echo of our own outbound publish (< 500ms)
    if (lastOutboundPublishTs[zoneIdx] > 0 && (now - lastOutboundPublishTs[zoneIdx] < LD_ECHO_SUPPRESS_MS)) {
        return;
    }

    char safeBuf[256];
    memcpy(safeBuf, payload, length);
    safeBuf[length] = '\0';

    // Exact self-echo guard: ignore Mosquitto echoing back our own retained publish regardless of delay!
    if (lastPublishedPayload[zoneIdx][0] != '\0' && strcmp(safeBuf, lastPublishedPayload[zoneIdx]) == 0) {
        return;
    }

    StaticJsonDocument<384> doc;
    if (deserializeJson(doc, safeBuf)) return;

    bool on     = doc["on"] | false;
    uint8_t bri = doc["bri"] | 255;

    // Extract un-multiplied raw 0-255 channels directly from MQTT JSON
    uint8_t raw_r  = doc["r"]  | 0;
    uint8_t raw_g  = doc["g"]  | 0;
    uint8_t raw_b  = doc["b"]  | 0;
    uint8_t raw_cw = doc["cw"] | 0;
    uint8_t raw_ww = doc["ww"] | 0;

    MilightCommand cmd;
    if (!on || bri == 0) {
        // Still compute underlying Hue/Kelvin from retained payload so state table is seeded
        cmd = translateDDP5ChannelToMilight(raw_r, raw_g, raw_b, raw_cw, raw_ww);
        cmd.power_on   = false;
        cmd.brightness = 0;
    } else {
        // Extract exact Mode, Hue, Saturation, and Warmth from unscaled 0..255 channels
        cmd = translateDDP5ChannelToMilight(raw_r, raw_g, raw_b, raw_cw, raw_ww);
        if (cmd.power_on) {
            // Compute brightness using identical integer scaling to wled_core.py send_static_ddp_frame()
            // so boot-seeded MQTT brightness matches subsequent DDP frames to the exact %!
            float scale = bri / 255.0f;
            uint8_t sr  = (uint8_t)(raw_r  * scale);
            uint8_t sg  = (uint8_t)(raw_g  * scale);
            uint8_t sb  = (uint8_t)(raw_b  * scale);
            uint8_t scw = (uint8_t)(raw_cw * scale);
            uint8_t sww = (uint8_t)(raw_ww * scale);
            MilightCommand scaledCmd = translateDDP5ChannelToMilight(sr, sg, sb, scw, sww);
            cmd.brightness = scaledCmd.power_on
                             ? scaledCmd.brightness
                             : max((uint8_t)1, (uint8_t)roundf((bri / 255.0f) * 100.0f));
        }
    }

    // On initial boot retained delivery, seed the internal 8-zone state table & Hub UI store
    if (!initializedFlags[zoneIdx]) {
        activeCmds[zoneIdx]       = cmd;
        pendingCmds[zoneIdx]      = cmd;
        initializedFlags[zoneIdx] = true;
        syncHubGroupStateFromCommand(zoneIdx, cmd);
        Serial.printf("[DDP-MQTT] Zone %d Boot-Seeded Retained State -> ON:%d Mode:%s Bri:%d%% Hue:%d° Sat:%d%% Warmth:%d%%\n",
                      zoneNum, cmd.power_on, cmd.is_white_mode ? "WHITE" : "COLOR",
                      cmd.brightness, cmd.hue, cmd.saturation, cmd.kelvin);
        return;
    }

    if (isCommandSignificantlyDifferent(zoneIdx, cmd)) {
        lastInboundSyncTs[zoneIdx] = now;
        strncpy(lastPublishedPayload[zoneIdx], safeBuf, sizeof(lastPublishedPayload[zoneIdx]) - 1);
        lastPublishedPayload[zoneIdx][sizeof(lastPublishedPayload[zoneIdx]) - 1] = '\0';
        markZoneDirty(zoneIdx, cmd);
        Serial.printf("[DDP-MQTT] Zone %d Inbound Sync -> ON:%d Mode:%s Bri:%d%% Hue:%d° Sat:%d%% Warmth:%d%%\n",
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
        markZoneDirty(zoneIdx, cmd);
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

void DDPStaticZoneHandler::markZoneDirty(uint8_t zoneIdx, const MilightCommand& cmd) {
    uint32_t now = millis();
    prevZoneDirtyGapMs[zoneIdx] = (lastZoneDirtyTs[zoneIdx] > 0) ? (now - lastZoneDirtyTs[zoneIdx]) : 9999;
    lastZoneDirtyTs[zoneIdx] = now;
    pendingCmds[zoneIdx] = cmd;
    dirtyFlags[zoneIdx] = true;
}

void DDPStaticZoneHandler::processDirtyZones() {
    uint32_t now = millis();

    // Dynamic inter-zone cadence: 18ms during active DDP streaming vs 45ms idle
    bool isStreaming = (lastDdpPacketTs > 0 && (now - lastDdpPacketTs < 250));
    uint32_t minGapMs = isStreaming ? 18 : 45;
    if (now - lastTxTs < minGapMs) return;

    // Priority 1: Fair round-robin scan of dirty zones
    for (uint8_t step = 0; step < NUM_FUT089_ZONES; step++) {
        uint8_t i = (rrZoneCursor + step) % NUM_FUT089_ZONES;
        if (dirtyFlags[i]) {
            const MilightCommand& target = pendingCmds[i];
            const MilightCommand& prev   = activeCmds[i];
            bool powerTransition = (!initializedFlags[i]) || (target.power_on != prev.power_on);
            bool rapidMotion = (!powerTransition) && target.power_on && (prevZoneDirtyGapMs[i] <= DDP_MOTION_WINDOW_MS);

            if (rapidMotion) {
                commitZoneState(i, DDP_FAST_REPEATS, false);
                anchorPendingFlags[i] = true;
                fastBurstsSent++;
            } else {
                commitZoneState(i, 0, false);
                anchorPendingFlags[i] = false;
            }

            activeCmds[i]       = target;
            initializedFlags[i] = true;
            dirtyFlags[i]       = false;
            lastTxTs            = now;
            rrZoneCursor        = (i + 1) % NUM_FUT089_ZONES;
            return;
        }
    }

    // Priority 2: Tier 2 Settle Anchor Burst (fires 150ms after rapid motion stops on a zone!)
    for (uint8_t step = 0; step < NUM_FUT089_ZONES; step++) {
        uint8_t i = (rrZoneCursor + step) % NUM_FUT089_ZONES;
        if (anchorPendingFlags[i] && (now - lastZoneDirtyTs[i] >= DDP_ANCHOR_SETTLE_MS)) {
            anchorPendingFlags[i] = false;
            if (activeCmds[i].power_on) {
                pendingCmds[i] = activeCmds[i];
                commitZoneState(i, DDP_ANCHOR_REPEATS, true);
                anchorBurstsSent++;
                lastTxTs     = now;
                rrZoneCursor = (i + 1) % NUM_FUT089_ZONES;
                return;
            }
        }
    }
}

void DDPStaticZoneHandler::commitZoneState(uint8_t zoneIndex, size_t repeatsOverride, bool forceFullSync) {
    uint8_t zoneId = zoneIndex + 1;
    MilightCommand& target = pendingCmds[zoneIndex];
    const MilightCommand& prev = activeCmds[zoneIndex];
    bool firstRun = !initializedFlags[zoneIndex];

    // Enforce 10%..100% brightness floor whenever ON
    if (target.power_on && target.brightness < 10) {
        target.brightness = 10;
    }

    // Zone 7 ('Fence Lights') is a 4-channel RGBW LED controller paired to FUT089:
    // Desaturated colors (< 25% sat) should switch cleanly to pure White LED instead of lighting Red+White together!
    bool isRgbwFence = (zoneId == 7);
    if (isRgbwFence && !target.is_white_mode) {
        if (target.saturation < 25) {
            target.is_white_mode = true;
        } else {
            target.saturation = 100;
        }
    }

    isInternalTx = true;
    lastInternalCommitTs[zoneIndex] = millis();

    milightClient->setInternalOrigin(true);
    if (repeatsOverride > 0) {
        milightClient->setRepeatsOverride(repeatsOverride);
    } else {
        milightClient->clearRepeatsOverride();
    }

    milightClient->prepare(MiLightRemoteType::REMOTE_TYPE_FUT089, defaultDeviceId, zoneId);

    // 1. Handle OFF
    if (!target.power_on) {
        if (firstRun || prev.power_on || forceFullSync) {
            milightClient->updateStatus(MiLightStatus::OFF);
            Serial.printf("[DDP-HUB] Zone %d -> OFF\n", zoneId);
        }
        milightClient->clearRepeatsOverride();
        milightClient->setInternalOrigin(false);
        isInternalTx = false;
        return;
    }

    // 2. Wake bulb if transitioning from OFF -> ON
    bool justTurnedOn = firstRun || !prev.power_on;
    bool modeChanged  = firstRun || justTurnedOn || forceFullSync || (target.is_white_mode != prev.is_white_mode);

    if (justTurnedOn) {
        milightClient->updateStatus(MiLightStatus::ON);
    }

    // 3. White Mode transmission
    if (target.is_white_mode) {
        bool briChanged = modeChanged || (abs((int)target.brightness - (int)prev.brightness) >= 2);
        if (isRgbwFence) {
            // 4-channel RGBW controller ONLY supports SET_WHITE (do NOT send FUT089_KELVIN opcode 0x07!)
            if (modeChanged) {
                milightClient->updateColorWhite();
            }
        } else {
            bool kelvinChanged = modeChanged || (abs((int)target.kelvin - (int)prev.kelvin) >= 2);
            if (modeChanged && !kelvinChanged) {
                milightClient->updateColorWhite();
            }
            if (kelvinChanged) {
                milightClient->updateTemperature(target.kelvin);
            }
        }
        if (briChanged) {
            milightClient->updateBrightness(target.brightness);
        }

        milightClient->clearRepeatsOverride();
        milightClient->setInternalOrigin(false);
        isInternalTx = false;
        return;
    }

    // 4. Color Mode transmission
    bool hueChanged = modeChanged || (hueCircularDiff360(target.hue, prev.hue) >= 2);
    bool satChanged = !isRgbwFence && (modeChanged || (abs((int)target.saturation - (int)prev.saturation) >= 2));
    bool briChanged = modeChanged || (abs((int)target.brightness - (int)prev.brightness) >= 2);

    if (hueChanged) {
        milightClient->updateHue(target.hue);
    }
    if (satChanged) {
        // Skip FUT089_SATURATION (opcode 0x07) on 4-channel RGBW Fence controller so White LED never mixes with Red!
        milightClient->updateSaturation(target.saturation);
    }
    if (briChanged) {
        milightClient->updateBrightness(target.brightness);
    }

    milightClient->clearRepeatsOverride();
    milightClient->setInternalOrigin(false);
    isInternalTx = false;
}

bool DDPStaticZoneHandler::publishRawMqtt(const char* topic, const char* payload, bool retain) {
    if (!mqtt.connected()) {
        return false;
    }
    return mqtt.publish(topic, payload, retain);
}
