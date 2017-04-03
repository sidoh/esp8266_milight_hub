#include <WebServer.h>
#include <MiLightClient.h>
#include <Settings.h>

#ifndef _MILIGHT_HTTP_SERVER
#define _MILIGHT_HTTP_SERVER 

#define MAX_DOWNLOAD_ATTEMPTS 3

typedef std::function<void(void)> SettingsSavedHandler;

const char DEFAULT_INDEX_PAGE[] PROGMEM
  = "Web app not installed. Click <a href=\"/download_update/web\">here</a> to attempt to download it from GitHub.";

class MiLightHttpServer {
public:
  MiLightHttpServer(Settings& settings, MiLightClient*& milightClient)
    : server(WebServer(80)),
      milightClient(milightClient),
      settings(settings)
  { 
    this->applySettings(settings);
  }
  
  void begin();
  void handleClient();
  void onSettingsSaved(SettingsSavedHandler handler);
  
protected:
  ESP8266WebServer::THandlerFunction handleServeFile(
    const char* filename, 
    const char* contentType, 
    const char* defaultText = NULL); 
    
  bool serveFile(const char* file, const char* contentType = "text/html");
  ESP8266WebServer::THandlerFunction handleUpdateFile(const char* filename);
  void applySettings(Settings& settings);
  
  void handleUpdateSettings();
  void handleGetRadioConfigs();
  void handleAbout();
  void handleGetLatestRelease();
  void handleSystemPost();
  void handleListenGateway(const UrlTokenBindings* urlBindings);
  void handleSendRaw(const UrlTokenBindings* urlBindings);
  void handleUpdateGroup(const UrlTokenBindings* urlBindings);
  void handleDownloadUpdate(const UrlTokenBindings* urlBindings);
  
  File updateFile;
  
  WebServer server;
  Settings& settings;
  MiLightClient*& milightClient;
  SettingsSavedHandler settingsSavedHandler;
  
};

#endif