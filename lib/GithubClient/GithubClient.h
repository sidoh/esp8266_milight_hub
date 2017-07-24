#include <Arduino.h>
#include <WiFiClientSecure.h>

#ifndef _GITHUB_CLIENT
#define _GITHUB_CLIENT

#define GITHUB_CLIENT_BUFFER_SIZE 32

// #define GITHUB_RAW_FINGERPRINT "CC AA 48 48 66 46 0E 91 53 2C 9C 7C 23 2A B1 74 4D 29 9D 33"
#define GITHUB_RAW_DOMAIN "raw.githubusercontent.com"

// #define GITHUB_API_FINGERPRINT "35 85 74 EF 67 35 A7 CE 40 69 50 F3 C0 F6 80 CF 80 3B 2E 19"
#define GITHUB_API_DOMAIN "api.github.com"

class GithubClient {
public:
  GithubClient(const char* domain)
    : domain(String(domain))
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
};

#endif
