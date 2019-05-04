#include <RichHttpServer.h>
#include <MiLightClient.h>
#include <Settings.h>
#include <WebSocketsServer.h>
#include <GroupStateStore.h>

#ifndef _MILIGHT_HTTP_SERVER
#define _MILIGHT_HTTP_SERVER

#define MAX_DOWNLOAD_ATTEMPTS 3

#ifndef MILIGHT_HTTP_JSON_BUFFER_SIZE
#define MILIGHT_HTTP_JSON_BUFFER_SIZE 4096
#endif

typedef std::function<void(void)> SettingsSavedHandler;
typedef std::function<void(const BulbId& id)> GroupDeletedHandler;

const char TEXT_PLAIN[] PROGMEM = "text/plain";
const char APPLICATION_JSON[] = "application/json";

class MiLightHttpServer {
public:
  MiLightHttpServer(Settings& settings, MiLightClient*& milightClient, GroupStateStore*& stateStore)
    : server(80),
      wsServer(WebSocketsServer(81)),
      numWsClients(0),
      milightClient(milightClient),
      settings(settings),
      stateStore(stateStore)
  {
    this->applySettings(settings);
  }

  void begin();
  void handleClient();
  void onSettingsSaved(SettingsSavedHandler handler);
  void onGroupDeleted(GroupDeletedHandler handler);
  void on(const char* path, HTTPMethod method, ESP8266WebServer::THandlerFunction handler);
  void handlePacketSent(uint8_t* packet, const MiLightRemoteConfig& config);
  WiFiClient client();

protected:

  bool serveFile(const char* file, const char* contentType = "text/html");
  void handleServe_P(const char* data, size_t length);
  void applySettings(Settings& settings);
  void sendGroupState(BulbId& bulbId, GroupState* state, RichHttp::Response& response);

  void serveSettings();
  void handleUpdateSettings(JsonDocument& request, RichHttp::Response& response);
  void handleUpdateSettingsPost();
  void handleUpdateFile(const char* filename);

  void handleGetRadioConfigs(RichHttp::Response& response);

  void handleAbout(RichHttp::Response& response);
  void handleSystemPost(JsonDocument& request, RichHttp::Response& response);
  void handleFirmwareUpload();
  void handleFirmwarePost();
  void handleListenGateway(const UrlTokenBindings* urlBindings, RichHttp::Response& response);
  void handleSendRaw(const UrlTokenBindings* urlBindings, JsonDocument& request, RichHttp::Response& response);
  void handleUpdateGroup(const UrlTokenBindings* urlBindings, JsonDocument& request, RichHttp::Response& response);
  void handleDeleteGroup(const UrlTokenBindings* urlBindings, RichHttp::Response& response);
  void handleGetGroup(const UrlTokenBindings* urlBindings, RichHttp::Response& response);

  void handleRequest(const JsonObject& request);
  void handleWsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

  File updateFile;

  RichHttpServer<RichHttp::Generics::Configs::EspressifBuiltin> server;
  WebSocketsServer wsServer;
  size_t numWsClients;
  MiLightClient*& milightClient;
  Settings& settings;
  GroupStateStore*& stateStore;
  SettingsSavedHandler settingsSavedHandler;
  GroupDeletedHandler groupDeletedHandler;
  ESP8266WebServer::THandlerFunction _handleRootPage;

};

#endif
