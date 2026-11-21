#ifndef UNIT_TEST

#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <cstdlib>
#include <FS.h>
#include <IntParsing.h>
#include <LinkedList.h>
#include <LEDStatus.h>
#include <GroupStateStore.h>
#include <MiLightRadioConfig.h>
#include <MiLightRemoteConfig.h>
#include <MiLightHttpServer.h>
#include <Settings.h>
#include <MiLightUdpServer.h>
#include <MqttClient.h>
#include <MiLightDiscoveryServer.h>
#include <MiLightClient.h>
#include <BulbStateUpdater.h>
#include <RadioSwitchboard.h>
#include <PacketSender.h>
#include <HomeAssistantDiscoveryClient.h>
#include <TransitionController.h>
#include <ProjectWifi.h>
#include <DDPStaticZoneHandler.h>

#include <ESPId.h>

#ifdef ESP8266
  #include <ESP8266mDNS.h>
  #include <ESP8266SSDP.h>
#elif ESP32
  #include "ESP32SSDP.h"
  #include <esp_wifi.h>
  #include <SPIFFS.h>
  #include <ESPmDNS.h>
  #include <SPI.h>
  #include <HspiNRF24MiLightRadio.h>
  #include <V2RFEncoding.h>
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>
  #include <freertos/queue.h>
#endif

#include <vector>
#include <memory>
#include "ProjectFS.h"
#include <RemoteRegistry.h>

WiFiManager* wifiManager;
WiFiManagerParameter* wifiStaticIP = NULL;
WiFiManagerParameter* wifiStaticIPNetmask = NULL;
WiFiManagerParameter* wifiStaticIPGateway = NULL;
WiFiManagerParameter* wifiMode = NULL;

static LEDStatus *ledStatus;

Settings settings;

MiLightClient* milightClient = NULL;
RadioSwitchboard* radios = nullptr;
RadioSwitchboard* rxRadios = nullptr;
PacketSender* packetSender = nullptr;
std::shared_ptr<MiLightRadioFactory> radioFactory;
#ifdef ESP32
struct Radio2ScanResult {
  bool scanned = false;
  int csnPin = -1;
  int cePin = -1;
  bool ceHardwiredHigh = false;
};
Radio2ScanResult g_radio2Scan;
std::shared_ptr<HspiNRF24Factory> rxRadioFactory;
uint32_t g_rxPacketsDecoded = 0;
uint32_t g_rxSelfEchoIgnored = 0;
uint32_t g_core0LoopTicks = 0;
String g_lastSniffedSummary = "none";
String g_lastClientCmdSummary = "none";

struct Core0RadioEvent {
  uint8_t packet[MILIGHT_MAX_PACKET_LENGTH];
  uint8_t packetLen;
  const MiLightRemoteConfig* remoteConfig;
  bool isFromRxSniffer;
  bool isInternalTx;
};

QueueHandle_t g_core0EventQueue = nullptr;
TaskHandle_t g_core0RadioTaskHandle = nullptr;
volatile bool g_pauseCore0Radio = false;
volatile bool g_core0RadioPausedAck = false;
#endif
MiLightHttpServer *httpServer = NULL;
MqttClient* mqttClient = NULL;
bool initialized = false;
MiLightDiscoveryServer* discoveryServer = NULL;
uint8_t currentRadioType = 0;

GroupStateStore* stateStore = NULL;
BulbStateUpdater* bulbStateUpdater = NULL;
TransitionController transitions;

std::vector<std::shared_ptr<MiLightUdpServer>> udpServers;
std::unique_ptr<DDPStaticZoneHandler> ddpServer;

void initMilightUdpServers() {
  if (! WiFi.isConnected()) {
    return;
  }

  udpServers.clear();

  for (size_t i = 0; i < settings.gatewayConfigs.size(); ++i) {
    const GatewayConfig& config = *settings.gatewayConfigs[i];

    std::shared_ptr<MiLightUdpServer> server = MiLightUdpServer::fromVersion(
      config.protocolVersion,
      milightClient,
      config.port,
      config.deviceId
    );

    if (server == NULL) {
      Serial.print(F("Error creating UDP server with protocol version: "));
      Serial.println(config.protocolVersion);
    } else {
      udpServers.push_back(std::move(server));
      udpServers[i]->begin();
    }
  }

  if (discoveryServer) {
    delete discoveryServer;
    discoveryServer = NULL;
  }
  if (settings.discoveryPort != 0) {
    discoveryServer = new MiLightDiscoveryServer(settings);
    discoveryServer->begin();
  }
}

void processDecodedPacket(uint8_t* packet, const MiLightRemoteConfig& config, bool isFromRxSniffer, bool isInternalTx) {
  StaticJsonDocument<200> buffer;
  JsonObject result = buffer.to<JsonObject>();

  const MiLightRemoteConfig* effectiveCfg = &config;
  if (config.type == REMOTE_TYPE_FUT089 && config.packetFormatter->getPacketLength() == 9) {
    uint8_t decTmp[9];
    memcpy(decTmp, packet, 9);
    V2RFEncoding::decodeV2Packet(decTmp);
    uint16_t peekDevId = ((uint16_t)decTmp[2] << 8) | decTmp[3];
    uint8_t peekCmd = decTmp[4] & 0x7F;
    MiLightRemoteType regType;
    if (peekCmd == 0x08 || (RemoteRegistry::instance().hasTypeOverride(peekDevId, regType) && regType == REMOTE_TYPE_FUT100)) {
      effectiveCfg = &FUT100Config;
    }
  }
  BulbId bulbId = effectiveCfg->packetFormatter->parsePacket(packet, result);

  ledStatus->oneshot(settings.ledModePacket, settings.ledModePacketCount);

  if (bulbId == DEFAULT_BULB_ID) {
    return;
  }

  const MiLightRemoteConfig& remoteConfig =
    *MiLightRemoteConfig::fromType(bulbId.deviceType);

  // Normalize wild V2 slider group bytes (e.g. Z83, Z223 on physical rgb_cct / fut091 / 1-zone remotes)
  bulbId.groupId = RemoteRegistry::instance().resolveGroupId(
    bulbId.deviceId,
    bulbId.groupId,
    (uint8_t)remoteConfig.numGroups
  );

  // If a packet changes brightness, hue, saturation, or kelvin without explicitly setting state,
  // the bulb implicitly turns ON (both on physical Mi-Light bulbs and in GroupStateStore)!
  if (!result.containsKey("state") && !result.containsKey("command") &&
      (result.containsKey("brightness") || result.containsKey("level") ||
       result.containsKey("hue") || result.containsKey("saturation") ||
       result.containsKey("color_temp") || result.containsKey("kelvin"))) {
    result["state"] = "ON";
  }

  // Always create/persist stateStore entry even on first touch of a zone!
  GroupState* groupState = stateStore->get(bulbId);
  const GroupState stateUpdates(groupState, result);
  if (groupState != NULL) {
    groupState->patch(stateUpdates);
    stateStore->set(bulbId, *groupState);
  } else {
    stateStore->set(bulbId, stateUpdates);
    groupState = stateStore->get(bulbId);
  }

  if (mqttClient) {
    char output[200];
    serializeJson(result, output);
    mqttClient->sendUpdate(remoteConfig, bulbId.deviceId, bulbId.groupId, output);

    if (groupState != NULL) {
      bulbStateUpdater->enqueueUpdate(bulbId, *groupState);
    }
  }

  if (httpServer) {
    httpServer->handlePacketSent(packet, remoteConfig, bulbId, result);
  }

  if (ddpServer) {
    if (isFromRxSniffer) {
#ifdef ESP32
      static uint32_t s_lastV2Sig = 0xFFFFFFFF;
      static uint32_t s_lastV2Ms = 0;
      if (remoteConfig.packetFormatter->getPacketLength() == 9) {
        uint8_t dec[9];
        memcpy(dec, packet, 9);
        V2RFEncoding::decodeV2Packet(dec);
        uint32_t sig = ((uint32_t)bulbId.deviceId << 16) | ((uint32_t)dec[6] << 8) | ((uint32_t)(dec[4] & 0x7F) ^ dec[5]);
        uint32_t nowMs = millis();
        if (sig == s_lastV2Sig && (nowMs - s_lastV2Ms < 110)) {
          return;
        }
        s_lastV2Sig = sig;
        s_lastV2Ms = nowMs;
      }
      g_rxPacketsDecoded++;
      char summary[80];
      snprintf(summary, sizeof(summary), "0x%04X %s grp=%u", bulbId.deviceId, remoteConfig.name.c_str(), bulbId.groupId);
      g_lastSniffedSummary = summary;
#endif
      ddpServer->publishSniffedRemoteEvent(bulbId.deviceId, bulbId.groupId, remoteConfig.name, result);
      if (remoteConfig.packetFormatter->getPacketLength() == 9) {
        uint8_t d9[9];
        memcpy(d9, packet, 9);
        V2RFEncoding::decodeV2Packet(d9);
        char v2Hex[96];
        snprintf(v2Hex, sizeof(v2Hex), "raw=%02X%02X%02X%02X%02X%02X%02X%02X%02X dec=%02X:%02X:%02X%02X:%02X:%02X:%02X:%02X:%02X", packet[0], packet[1], packet[2], packet[3], packet[4], packet[5], packet[6], packet[7], packet[8], d9[0], d9[1], d9[2], d9[3], d9[4], d9[5], d9[6], d9[7], d9[8]);
        result["v2_dec"] = v2Hex;
      }
      RemoteRegistry::instance().handlePacket(bulbId.deviceId, bulbId.groupId, remoteConfig.name, result);
    } else if (!isInternalTx) {
#ifdef ESP32
      char cmdSum[96];
      char resJson[64];
      serializeJson(result, resJson, sizeof(resJson));
      snprintf(cmdSum, sizeof(cmdSum), "0x%04X %s grp=%u %s", bulbId.deviceId, remoteConfig.name.c_str(), bulbId.groupId, resJson);
      g_lastClientCmdSummary = cmdSum;
#endif
    }

    // Publish to Light-Dancer MQTT for ALL external sources (Alexa, Apple Home, Hub UI, Physical Remotes),
    // while strictly skipping packets generated internally by DDPStaticZoneHandler itself!
    if (!isInternalTx) {
      const GroupState* updatedState = stateStore->get(bulbId);
      ddpServer->publishLocalStateToMqtt(bulbId.deviceId, bulbId.groupId, bulbId.deviceType, updatedState ? updatedState : &stateUpdates);
    }
  }
}

void onPacketSentHandler(uint8_t* packet, const MiLightRemoteConfig& config, bool isInternal) {
#ifdef ESP32
  if (g_core0EventQueue != nullptr && xPortGetCoreID() == 0) {
    Core0RadioEvent ev;
    size_t len = config.packetFormatter->getPacketLength();
    if (len > MILIGHT_MAX_PACKET_LENGTH) len = MILIGHT_MAX_PACKET_LENGTH;
    memcpy(ev.packet, packet, len);
    ev.packetLen = (uint8_t)len;
    ev.remoteConfig = &config;
    ev.isFromRxSniffer = false;
    ev.isInternalTx = isInternal;
    xQueueSend(g_core0EventQueue, &ev, 0);
    return;
  }
#endif
  processDecodedPacket(packet, config, false, isInternal);
}

void pollRadioListenOnce() {
  if (! settings.listenRepeats) {
    return;
  }

  RadioSwitchboard* activeRx = (rxRadios != nullptr) ? rxRadios : radios;
  if (activeRx == nullptr) {
    return;
  }

  // Only pause listening if sharing a single transceiver with TX
  if (rxRadios == nullptr && packetSender->isSending()) {
    return;
  }

  static uint32_t lastProtoSwitchMs = 0;
  static size_t heldConfigIx = 2;
  size_t targetConfigIx = heldConfigIx;

  if (rxRadios != nullptr && activeRx->getNumRadios() >= 3) {
    // Keep Radio #2 locked 100% onto Config 2 (V2 2.4GHz syncword 0x7236/0x1809: FUT089, B8, FUT092, FUT096, B4, T4, FUT100)
    // unless a legacy V1 remote (rgbw/cct/fut020) is explicitly registered in RemoteRegistry!
    bool hasLegacyRemote = false;
    for (const auto& rr : RemoteRegistry::instance().remotes) {
      if (rr.active && (rr.remoteType == "rgbw" || rr.remoteType == "cct" || rr.remoteType == "rgb" || rr.remoteType == "fut020")) {
        hasLegacyRemote = true;
        break;
      }
    }
    if (!hasLegacyRemote) {
      targetConfigIx = 2;
    } else {
      uint32_t nowMs = millis();
      uint32_t dwellMs = (heldConfigIx == 2) ? 450 : 12;
      if (nowMs - lastProtoSwitchMs >= dwellMs) {
        lastProtoSwitchMs = nowMs;
        uint8_t slot = currentRadioType++;
        if ((slot & 0x03) != 0x03) heldConfigIx = 2;
        else {
          static uint8_t altCursor = 0;
          const size_t altIndices[] = { 0, 1, 3, 4 };
          heldConfigIx = altIndices[altCursor++ & 0x03] % activeRx->getNumRadios();
        }
      }
      targetConfigIx = heldConfigIx;
    }
  } else {
    targetConfigIx = currentRadioType++ % activeRx->getNumRadios();
  }

  std::shared_ptr<MiLightRadio> radio = activeRx->switchRadio(targetConfigIx);
  if (radio == nullptr) {
    return;
  }

  size_t polls = (rxRadios != nullptr) ? max((size_t)6, (size_t)settings.listenRepeats) : settings.listenRepeats;

  for (size_t i = 0; i < polls; i++) {
    if (activeRx->available()) {
      uint8_t readPacket[MILIGHT_MAX_PACKET_LENGTH];
      size_t packetLen = activeRx->read(readPacket);

      if (rxRadios != nullptr && packetSender != nullptr && packetSender->isRecentLocalTx(readPacket, packetLen)) {
#ifdef ESP32
        g_rxSelfEchoIgnored++;
#endif
        continue;
      }

      const MiLightRemoteConfig* remoteConfig = MiLightRemoteConfig::fromReceivedPacket(
        radio->config(),
        readPacket,
        packetLen
      );

      if (remoteConfig == NULL) {
        continue;
      }

#ifdef ESP32
      if (g_core0EventQueue != nullptr && xPortGetCoreID() == 0) {
        Core0RadioEvent ev;
        size_t copyLen = (packetLen <= MILIGHT_MAX_PACKET_LENGTH) ? packetLen : MILIGHT_MAX_PACKET_LENGTH;
        memcpy(ev.packet, readPacket, copyLen);
        ev.packetLen = (uint8_t)copyLen;
        ev.remoteConfig = remoteConfig;
        ev.isFromRxSniffer = true;
        ev.isInternalTx = false;
        xQueueSend(g_core0EventQueue, &ev, 0);
        continue;
      }
#endif
      processDecodedPacket(readPacket, *remoteConfig, true, false);
    }
  }
}

#ifdef ESP32
void drainCore0RadioEvents() {
  if (g_core0EventQueue == nullptr) return;
  Core0RadioEvent ev;
  uint8_t drained = 0;
  while (drained < 16 && xQueueReceive(g_core0EventQueue, &ev, 0) == pdTRUE) {
    if (ev.remoteConfig != nullptr) {
      processDecodedPacket(ev.packet, *ev.remoteConfig, ev.isFromRxSniffer, ev.isInternalTx);
    }
    drained++;
  }
}

void core0RadioTask(void* param) {
  for (;;) {
    if (g_pauseCore0Radio) {
      g_core0RadioPausedAck = true;
      vTaskDelay(pdMS_TO_TICKS(2));
      continue;
    }
    g_core0RadioPausedAck = false;

    bool didWork = false;
    if (packetSender != nullptr) {
      if (packetSender->isSending()) didWork = true;
      packetSender->loop();
    }
    if (radios != nullptr || rxRadios != nullptr) {
      pollRadioListenOnce();
    }
    g_core0LoopTicks++;

    if (!didWork) {
      vTaskDelay(1);
    } else {
      taskYIELD();
    }
  }
}
#endif

void onUpdateBegin() {
  if (bulbStateUpdater) {
    bulbStateUpdater->disable();
  }
}

void onUpdateEnd() {
  if (bulbStateUpdater) {
    bulbStateUpdater->enable();
  }
}

void applySettings() {
#ifdef ESP32
  if (g_core0RadioTaskHandle != nullptr) {
    g_pauseCore0Radio = true;
    uint32_t waitStart = millis();
    while (!g_core0RadioPausedAck && (millis() - waitStart < 100)) {
      delay(1);
    }
  }
#endif
  if (milightClient) {
    delete milightClient;
  }
  if (mqttClient) {
    delete mqttClient;
    delete bulbStateUpdater;

    mqttClient = NULL;
    bulbStateUpdater = NULL;
  }
  if (stateStore) {
    delete stateStore;
  }
  if (packetSender) {
    delete packetSender;
  }
  if (radios) {
    delete radios;
    radios = nullptr;
  }
  if (rxRadios) {
    delete rxRadios;
    rxRadios = nullptr;
  }

  transitions.setDefaultPeriod(settings.defaultTransitionPeriod);
  radioFactory = MiLightRadioFactory::fromSettings(settings);

  if (radioFactory == NULL) {
    Serial.println(F("ERROR: unable to construct radio factory"));
  }

  stateStore = new GroupStateStore(MILIGHT_MAX_STATE_ITEMS, settings.stateFlushInterval);
  radios = new RadioSwitchboard(radioFactory, stateStore, settings);

#ifdef ESP32
  if (g_radio2Scan.csnPin >= 0) {
    // Live Over-The-Air Loopback Calibration using Radio 1 (VSPI) -> Radio 2 (HSPI)!
    // Tests every safe GPIO & both syncword byte orders until Radio 2 physically receives Radio 1's Mi-Light frame!
    const uint8_t ceCandidates[] = { 4, 2, 16, 17, 27, 26, 25, 33, 32, 5 };
    uint8_t dummyFrame[9] = { 0x25, 0x12, 0x5C, 0x01, 0x00, 0x00, 0x00, 0x00, 0xAA };

    int verifiedCe = -1;
    for (int revOrder = 0; revOrder <= 1 && verifiedCe < 0; revOrder++) {
      HspiNRF24MiLightRadio::reverseSyncwordOrder = (revOrder == 1);

      for (uint8_t candCe : ceCandidates) {
        if (candCe == (uint8_t)g_radio2Scan.csnPin || candCe == settings.cePin || candCe == settings.csnPin || candCe == settings.ledPin) {
          continue;
        }
        // Pull all other candidates LOW so only candCe is HIGH
        for (uint8_t other : ceCandidates) {
          if (other == (uint8_t)g_radio2Scan.csnPin || other == settings.cePin || other == settings.csnPin || other == settings.ledPin) continue;
          pinMode(other, OUTPUT);
          digitalWrite(other, LOW);
        }

        auto testFactory = std::make_shared<HspiNRF24Factory>(candCe, (uint8_t)g_radio2Scan.csnPin);
        if (!testFactory->isReady()) break;

        auto testRx = testFactory->create(MiLightRadioConfig::ALL_CONFIGS[2]);
        testRx->begin(); // Sets Pipe 1 syncword, channel, and pulls candCe HIGH!
        delayMicroseconds(250);

        // Transmit 4 real V2 frames from Radio 1 (VSPI) across LOW/MID/HIGH channels
        radios->switchRadio(2);
        for (int burst = 0; burst < 4; burst++) {
          radios->write(dummyFrame, sizeof(dummyFrame));
        }
        delayMicroseconds(200);

        if (testRx->available()) {
          uint8_t rxBuf[12];
          size_t rxLen = sizeof(rxBuf);
          (rxLen = MILIGHT_MAX_PACKET_LENGTH, testRx->read(rxBuf, rxLen));
          verifiedCe = candCe;
          g_radio2Scan.cePin = candCe;
          break;
        }
        digitalWrite(candCe, LOW);
      }
    }

    // Release non-matching candidate pins back to INPUT
    for (uint8_t other : ceCandidates) {
      if (other == (uint8_t)g_radio2Scan.csnPin || other == settings.cePin || other == settings.csnPin || other == settings.ledPin) continue;
      if ((int)other != g_radio2Scan.cePin) {
        pinMode(other, INPUT);
      }
    }

    // Reset diagnostic counters after boot calibration burst
    HspiNRF24MiLightRadio::rawFifoHits = 0;
    HspiNRF24MiLightRadio::crcPassCount = 0;
    HspiNRF24MiLightRadio::crcFailCount = 0;

    if (g_radio2Scan.cePin >= 0) {
      rxRadioFactory = std::make_shared<HspiNRF24Factory>((uint8_t)g_radio2Scan.cePin, (uint8_t)g_radio2Scan.csnPin);
      if (rxRadioFactory->isReady()) {
        rxRadios = new RadioSwitchboard(rxRadioFactory, stateStore, settings);
      }
    }
  }
#endif
  packetSender = new PacketSender(*radios, settings, onPacketSentHandler);

  milightClient = new MiLightClient(
    *radios,
    *packetSender,
    stateStore,
    settings,
    transitions
  );
  milightClient->onUpdateBegin(onUpdateBegin);
  milightClient->onUpdateEnd(onUpdateEnd);

  if (settings._mqttServer.length() == 0) {
    settings._mqttServer = "192.168.6.196:1883";
    if (settings.mqttUpdateTopicPattern.length() == 0) {
      settings.mqttUpdateTopicPattern = "milight/updates/:hex_device_id/:device_type/:group_id";
    }
    settings.save();
  }
  if (initialized && settings.mqttServer().length() > 0) {
    mqttClient = new MqttClient(settings, milightClient);
    mqttClient->begin();
    mqttClient->onConnect([]() {
      RemoteRegistry::instance().publishDiscovery();
      if (settings.homeAssistantDiscoveryPrefix.length() > 0) {
        HomeAssistantDiscoveryClient discoveryClient(settings, mqttClient);
        discoveryClient.sendDiscoverableDevices(settings.groupIdAliases);
        discoveryClient.removeOldDevices(settings.deletedGroupIdAliases);

        settings.deletedGroupIdAliases.clear();
      }
    });

    bulbStateUpdater = new BulbStateUpdater(settings, *mqttClient, *stateStore);
  }

  RemoteRegistry::instance().setFallbackPublisher([](const char* t, const char* p, bool r) -> bool {
    return (ddpServer != nullptr) ? ddpServer->publishRawMqtt(t, p, r) : false;
  });
  RemoteRegistry::instance().begin(&mqttClient, &milightClient);
    if (mqttClient == NULL && settings.mqttServer().length() > 0) {
    mqttClient = new MqttClient(settings, milightClient);
    mqttClient->begin();
    mqttClient->onConnect([]() {
      RemoteRegistry::instance().publishDiscovery();
      if (settings.homeAssistantDiscoveryPrefix.length() > 0) {
        HomeAssistantDiscoveryClient discoveryClient(settings, mqttClient);
        discoveryClient.sendDiscoverableDevices(settings.groupIdAliases);
        discoveryClient.removeOldDevices(settings.deletedGroupIdAliases);
        settings.deletedGroupIdAliases.clear();
      }
    });
    if (bulbStateUpdater) {
      delete bulbStateUpdater;
    }
    bulbStateUpdater = new BulbStateUpdater(settings, *mqttClient, *stateStore);
  }
  initMilightUdpServers();

  if (ledStatus) {
    ledStatus->changePin(settings.ledPin);
    ledStatus->continuous(settings.ledModeOperating);
  }

  WiFi.hostname(settings.hostname);
#ifdef ESP8266
  WiFiPhyMode_t wifiPhyMode;
  switch (settings.wifiMode) {
    case WifiMode::B: wifiPhyMode = WIFI_PHY_MODE_11B; break;
    case WifiMode::G: wifiPhyMode = WIFI_PHY_MODE_11G; break;
    default:
    case WifiMode::N: wifiPhyMode = WIFI_PHY_MODE_11N; break;
  }
  WiFi.setPhyMode(wifiPhyMode);
#elif ESP32
  switch (settings.wifiMode) {
    case WifiMode::B: esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B); break;
    case WifiMode::G: esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11G); break;
    default:
    case WifiMode::N: esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11N); break;
  }
  esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20);
  g_pauseCore0Radio = false;
  if (g_core0EventQueue == nullptr) {
    g_core0EventQueue = xQueueCreate(32, sizeof(Core0RadioEvent));
  }
  if (g_core0RadioTaskHandle == nullptr && g_core0EventQueue != nullptr) {
    xTaskCreatePinnedToCore(
      core0RadioTask,
      "RadioCore0",
      6144,
      nullptr,
      3,
      &g_core0RadioTaskHandle,
      0
    );
  }
#endif
}

bool shouldRestart() {
  if (! settings.isAutoRestartEnabled()) {
    return false;
  }
  return settings.getAutoRestartPeriod()*60*1000 < millis();
}

void wifiExtraSettingsChange() {
  settings.wifiStaticIP = wifiStaticIP->getValue();
  settings.wifiStaticIPNetmask = wifiStaticIPNetmask->getValue();
  settings.wifiStaticIPGateway = wifiStaticIPGateway->getValue();
  settings.wifiMode = Settings::wifiModeFromString(wifiMode->getValue());
  settings.save();
  delay(1000);
  ESP.restart();
}

#ifdef ESP32

static uint8_t hspiReadReg(SPIClass& hspi, uint8_t csn, uint8_t reg) {
  hspi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(csn, LOW);
  delayMicroseconds(5);
  hspi.transfer(reg & 0x1F);
  uint8_t val = hspi.transfer(0xFF);
  digitalWrite(csn, HIGH);
  hspi.endTransaction();
  delayMicroseconds(5);
  return val;
}

static void hspiWriteReg(SPIClass& hspi, uint8_t csn, uint8_t reg, uint8_t val) {
  hspi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(csn, LOW);
  delayMicroseconds(5);
  hspi.transfer(0x20 | (reg & 0x1F));
  hspi.transfer(val);
  digitalWrite(csn, HIGH);
  hspi.endTransaction();
  delayMicroseconds(5);
}

static void hspiCmd(SPIClass& hspi, uint8_t csn, uint8_t cmd) {
  hspi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(csn, LOW);
  delayMicroseconds(5);
  hspi.transfer(cmd);
  digitalWrite(csn, HIGH);
  hspi.endTransaction();
  delayMicroseconds(5);
}

void scanSecondRadioPins() {
  const uint8_t rawCandidates[] = { 27, 26, 25, 33, 32, 15, 4, 5, 16, 17, 2 };
  std::vector<uint8_t> cands;
  for (uint8_t p : rawCandidates) {
    if (p == settings.cePin || p == settings.csnPin || p == settings.ledPin) continue;
    cands.push_back(p);
    pinMode(p, INPUT_PULLUP);
  }
  delay(2);

  SPIClass hspi(HSPI);
  hspi.begin(14, 12, 13, -1);

  for (uint8_t p : cands) {
    pinMode(p, OUTPUT);
    digitalWrite(p, HIGH);
    delayMicroseconds(10);

    hspiWriteReg(hspi, p, 0x05, 0x2A);
    uint8_t r1 = hspiReadReg(hspi, p, 0x05);
    hspiWriteReg(hspi, p, 0x05, 0x15);
    uint8_t r2 = hspiReadReg(hspi, p, 0x05);

    if (r1 == 0x2A && r2 == 0x15) {
      hspiWriteReg(hspi, p, 0x05, 0x02);
      g_radio2Scan.csnPin = p;
      digitalWrite(p, HIGH);
      break;
    }
    pinMode(p, INPUT_PULLUP);
  }

  if (g_radio2Scan.csnPin >= 0) {
    uint8_t csn = (uint8_t)g_radio2Scan.csnPin;
    for (uint8_t q : cands) {
      if (q != csn) pinMode(q, INPUT_PULLDOWN);
    }
    delay(2);

    hspiWriteReg(hspi, csn, 0x00, 0x0A); // PWR_UP=1, EN_CRC=1, PRIM_RX=0 (PTX)
    delay(5);                            // Crystal oscillator startup
    hspiWriteReg(hspi, csn, 0x01, 0x00); // EN_AA=0 (no ACK needed for TX_DS)
    hspiWriteReg(hspi, csn, 0x04, 0x00); // SETUP_RETR=0
    hspiWriteReg(hspi, csn, 0x05, 0x7C); // Channel 124 (2.524 GHz)
    hspiWriteReg(hspi, csn, 0x06, 0x00); // -18 dBm min power, 1 Mbps
    hspiCmd(hspi, csn, 0xE1);            // FLUSH_TX
    hspiWriteReg(hspi, csn, 0x07, 0x70); // Clear STATUS flags

    // Queue 4-byte dummy TX payload
    hspi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(csn, LOW);
    delayMicroseconds(5);
    hspi.transfer(0xA0);
    for (int i = 0; i < 4; i++) hspi.transfer(0x55);
    digitalWrite(csn, HIGH);
    hspi.endTransaction();

    delayMicroseconds(600);
    if (hspiReadReg(hspi, csn, 0x07) & 0x20) {
      g_radio2Scan.ceHardwiredHigh = true;
    } else {
      for (uint8_t q : cands) {
        if (q == csn) continue;
        pinMode(q, OUTPUT);
        digitalWrite(q, HIGH);
        delayMicroseconds(30);
        digitalWrite(q, LOW);
        delayMicroseconds(600);

        uint8_t st = hspiReadReg(hspi, csn, 0x07);
        uint8_t fifo = hspiReadReg(hspi, csn, 0x17);
        if ((st & 0x20) || (fifo & 0x10)) {
          g_radio2Scan.cePin = q;
          break;
        }
        pinMode(q, INPUT_PULLDOWN);
      }
    }

    // Power down cleanly & release unused test pins
    hspiCmd(hspi, csn, 0xE1);
    hspiWriteReg(hspi, csn, 0x07, 0x70);
    hspiWriteReg(hspi, csn, 0x00, 0x08);
    for (uint8_t q : cands) {
      if (q == csn) {
        pinMode(q, OUTPUT);
        digitalWrite(q, HIGH);
      } else if ((int)q == g_radio2Scan.cePin) {
        pinMode(q, OUTPUT);
        digitalWrite(q, LOW);
      } else {
        pinMode(q, INPUT);
      }
    }
  } else {
    for (uint8_t q : cands) pinMode(q, INPUT);
  }

  hspi.end();
  g_radio2Scan.scanned = true;
}
#endif

void aboutHandler(JsonDocument& json) {
  JsonObject mqtt = json.createNestedObject(FPSTR("mqtt"));
  mqtt[FPSTR("configured")] = (mqttClient != nullptr);

  if (mqttClient) {
    mqtt[FPSTR("connected")] = mqttClient->isConnected();
    mqtt[FPSTR("status")] = mqttClient->getConnectionStatusString();
  }
#ifdef ESP32
  JsonObject r2 = json.createNestedObject("radio2");
  r2["scanned"] = g_radio2Scan.scanned;
  r2["dual_rx_active"] = (rxRadios != nullptr);
  r2["csn_pin"] = g_radio2Scan.csnPin;
  r2["ce_pin"] = g_radio2Scan.cePin;
  r2["rev_syncword"] = HspiNRF24MiLightRadio::reverseSyncwordOrder;
  r2["raw_fifo_hits"] = HspiNRF24MiLightRadio::rawFifoHits;
  r2["crc_pass"] = HspiNRF24MiLightRadio::crcPassCount;
  r2["crc_fail"] = HspiNRF24MiLightRadio::crcFailCount;
  r2["rx_packets_decoded"] = g_rxPacketsDecoded;
  r2["self_echo_ignored"] = g_rxSelfEchoIgnored;
  r2["last_sniffed"] = g_lastSniffedSummary;
  r2["last_client_cmd"] = g_lastClientCmdSummary;
  r2["core0_task_active"] = (g_core0RadioTaskHandle != nullptr);
  r2["core0_ticks"] = g_core0LoopTicks;
  r2["tx_coalesced"] = packetSender ? packetSender->coalescedPackets() : 0;
  r2["ddp_fast_bursts"] = ddpServer ? ddpServer->getFastBurstsSent() : 0;
  r2["ddp_anchor_bursts"] = ddpServer ? ddpServer->getAnchorBurstsSent() : 0;
#endif
}

void onGroupDeleted(const BulbId& id) {
  if (mqttClient != NULL) {
    mqttClient->sendState(
      *MiLightRemoteConfig::fromType(id.deviceType),
      id.deviceId,
      id.groupId,
      ""
    );
  }
}

void postConnectSetup() {
  if (initialized) return;
  initialized = true;
  if (settings._mqttServer.length() == 0) {
    settings._mqttServer = "192.168.6.196:1883";
    if (settings.mqttUpdateTopicPattern.length() == 0) {
      settings.mqttUpdateTopicPattern = "milight/updates/:hex_device_id/:device_type/:group_id";
    }
    settings.save();
  }
  applySettings();

  delete wifiManager;
  wifiManager = NULL;

  MDNS.addService("http", "tcp", 80);

  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("ESP8266 MiLight Gateway");
  SSDP.setSerialNumber(getESPId());
  SSDP.setURL("/");
  SSDP.setDeviceType("upnp:rootdevice");
  SSDP.begin();

  httpServer = new MiLightHttpServer(settings, milightClient, stateStore, packetSender, radios, transitions);
  httpServer->onSettingsSaved(applySettings);
  httpServer->onGroupDeleted(onGroupDeleted);
  httpServer->onAbout(aboutHandler);
  httpServer->on("/description.xml", HTTP_GET, []() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    SSDP.schema(httpServer->client());
#pragma GCC diagnostic pop
  });
  httpServer->begin();

  transitions.addListener(
      [](const BulbId& bulbId, GroupStateField field, uint16_t value) {
          StaticJsonDocument<100> buffer;
          const char* fieldName = GroupStateFieldHelpers::getFieldName(field);
          buffer[fieldName] = value;
          milightClient->prepare(bulbId.deviceType, bulbId.deviceId, bulbId.groupId);
          milightClient->update(buffer.as<JsonObject>());
      }
  );

  initMilightUdpServers();

  if (milightClient != NULL) {
    uint16_t resolvedFut089Id = 0x4700;

    for (size_t g = 0; g < settings.gatewayConfigs.size(); ++g) {
      if (settings.gatewayConfigs[g]->protocolVersion == 6 ||
          settings.gatewayConfigs[g]->protocolVersion == MiLightRemoteType::REMOTE_TYPE_FUT089) {
        resolvedFut089Id = settings.gatewayConfigs[g]->deviceId;
        break;
      }
    }

    ddpServer = std::unique_ptr<DDPStaticZoneHandler>(new DDPStaticZoneHandler(milightClient, resolvedFut089Id, stateStore));
    ddpServer->begin();
  }

  Serial.printf_P(PSTR("Setup complete (version %s)\n"), QUOTE(MILIGHT_HUB_VERSION));
}

void setup() {
  Serial.begin(9600);
#ifdef ESP32
  WiFi.mode(WIFI_STA); // init LwIP mbox before any Settings/HTTP/MQTT/WiFi calls
#endif
  String ssid = "ESP" + String(getESPId());

  #ifdef ESP8266
    if (! ProjectFS.begin()) {
      Serial.println(F("Failed to mount file system"));
    }
  #else
    if (! ProjectFS.begin(true)) {
      Serial.println(F("Failed to mount file system"));
    }
  #endif

    pinMode(0, INPUT_PULLUP);
  delay(100);
  if (digitalRead(0) == LOW) {
    Serial.println(F("[SAFE-BOOT] BOOT button held! Deleting corrupt /config.json..."));
    ProjectFS.remove("/config.json");
    delay(1000);
  }
  Settings::load(settings);
#ifdef ESP32
  scanSecondRadioPins();
#endif
  
  ESPMH_SETUP_WIFI(settings);
  applySettings();

  ledStatus = new LEDStatus(settings.ledPin);
  ledStatus->continuous(settings.ledModeWifiConfig);

  if (! MDNS.begin("milight-hub")) {
    Serial.println(F("Error setting up MDNS responder"));
  }

  wifiManager = new WiFiManager();
  wifiManager->setBreakAfterConfig(true);
  wifiManager->setSaveConfigCallback(wifiExtraSettingsChange);
  wifiManager->setConfigPortalBlocking(false);
  wifiManager->setConnectTimeout(20);
  wifiManager->setConnectRetries(5);

  wifiStaticIP = new WiFiManagerParameter(
    "staticIP",
    "Static IP (Leave blank for dhcp)",
    settings.wifiStaticIP.c_str(),
    MAX_IP_ADDR_LEN
  );
  wifiManager->addParameter(wifiStaticIP);

  wifiStaticIPNetmask = new WiFiManagerParameter(
    "netmask",
    "Netmask (required if IP given)",
    settings.wifiStaticIPNetmask.c_str(),
    MAX_IP_ADDR_LEN
  );
  wifiManager->addParameter(wifiStaticIPNetmask);

  wifiStaticIPGateway = new WiFiManagerParameter(
    "gateway",
    "Default Gateway (optional, only used if static IP)",
    settings.wifiStaticIPGateway.c_str(),
    MAX_IP_ADDR_LEN
  );
  wifiManager->addParameter(wifiStaticIPGateway);

  wifiMode = new WiFiManagerParameter(
    "wifiMode",
    "WiFi Mode (b/g/n)",
    settings.wifiMode == WifiMode::B ? "b" : settings.wifiMode == WifiMode::G ? "g" : "n",
    1
  );
  wifiManager->addParameter(wifiMode);

  if (settings.wifiStaticIP.length() > 0) {
    Serial.printf_P(PSTR("We have a static IP: %s\n"), settings.wifiStaticIP.c_str());
    IPAddress _ip, _subnet, _gw;
    _ip.fromString(settings.wifiStaticIP);
    _subnet.fromString(settings.wifiStaticIPNetmask);
    _gw.fromString(settings.wifiStaticIPGateway);
    wifiManager->setSTAStaticIPConfig(_ip,_gw,_subnet);
  }

  wifiManager->setConfigPortalTimeout(180);
  wifiManager->setConfigPortalTimeoutCallback([]() {
      ledStatus->continuous(settings.ledModeWifiFailed);
      Serial.println(F("Wifi config portal timed out.  Restarting..."));
      delay(10000);
      ESP.restart();
  });

  if (wifiManager->autoConnect(ssid.c_str(), "milightHub")) {
    ledStatus->continuous(settings.ledModeOperating);
    Serial.println(F("Wifi connected succesfully\n"));
    WiFi.mode(WIFI_STA);
    postConnectSetup();
  }
}

void loop() {
  ledStatus->handle();

  if (shouldRestart()) {
    Serial.println(F("Auto-restart triggered. Restarting..."));
    ESP.restart();
  }

  if (wifiManager) {
    wifiManager->process();
  }

  if (WiFi.getMode() == WIFI_STA && WiFi.isConnected()) {
    postConnectSetup();

    httpServer->handleClient();
    if (mqttClient) {
      mqttClient->handleClient();
      bulbStateUpdater->loop();
    }

    for (auto & udpServer : udpServers) {
      udpServer->handleClient();
    }

    if (ddpServer) {
      ddpServer->loop();
    }
    RemoteRegistry::instance().tick();

    if (discoveryServer) {
      discoveryServer->handleClient();
    }

#ifdef ESP32
    if (g_core0RadioTaskHandle != nullptr) {
      drainCore0RadioEvents();
    } else {
      pollRadioListenOnce();
      packetSender->loop();
    }
#else
    pollRadioListenOnce();
    packetSender->loop();
#endif
    stateStore->limitedFlush();
    transitions.loop();
  }
}

#endif
