#include <Arduino.h>
#include <WiFiClientSecure.h>

#ifndef _GITHUB_DOWNLOADER
#define _GITHUB_DOWNLOADER 

#define GITHUB_DOWNLOADER_BUFFER_SIZE 100

#define GITHUB_SSL_FINGERPRINT "21 99 13 84 63 72 17 13 B9 ED 0E 8F 00 A5 9B 73 0D D0 56 58"
#define GITHUB_RAW_DOMAIN "raw.githubusercontent.com"

class GithubDownloader {
public:
  Stream& streamFile(const String& path);
  Stream& streamFile(const String& username, const String& repo, const String& path);
  
  bool downloadFile(const String& path, Stream& dest);
  bool downloadFile(const String& username, const String& repo, const String& path, Stream& dest);
  bool downloadFile(const String& username, const String& repo, const String& path, const String& fsPath);
  
  uint8_t buffer[GITHUB_DOWNLOADER_BUFFER_SIZE];
  
private:
  WiFiClientSecure client;
  
  String buildPath(const String& username, const String& repo, const String& path);
};

#endif