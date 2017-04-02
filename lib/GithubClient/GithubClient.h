#include <Arduino.h>
#include <WiFiClientSecure.h>

#ifndef _GITHUB_CLIENT
#define _GITHUB_CLIENT 

#define GITHUB_CLIENT_BUFFER_SIZE 32

#define GITHUB_RAW_FINGERPRINT "21 99 13 84 63 72 17 13 B9 ED 0E 8F 00 A5 9B 73 0D D0 56 58"
#define GITHUB_RAW_DOMAIN "raw.githubusercontent.com"

#define GITHUB_API_FINGERPRINT "35 85 74 EF 67 35 A7 CE 40 69 50 F3 C0 F6 80 CF 80 3B 2E 19"
#define GITHUB_API_DOMAIN "api.github.com"

class GithubClient {
public:
  GithubClient(const char* domain, const char* sslFingerprint)
    : domain(String(domain)),
      sslFingerprint(String(sslFingerprint))
  { }
  
  Stream& stream(const String& path);
  bool download(const String& path, Stream& dest);
  bool download(const String& path, const String& fsPath);
  
  static GithubClient rawDownloader();
  static GithubClient apiClient();
  
  static String buildRepoPath(const String& username, const String& repo, const String& path);
  static String buildApiRequest(const String& username, const String& repo, const String& path);
  
  uint8_t buffer[GITHUB_CLIENT_BUFFER_SIZE];
  
private:
  WiFiClientSecure client;
  const String domain;
  const String sslFingerprint;
};

#endif