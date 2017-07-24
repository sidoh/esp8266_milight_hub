#include <GithubClient.h>
#include <FS.h>
#include <Size.h>

static const char HTTP_REQUEST_FORMAT[] PROGMEM =
  "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: esp8266_milight_hub\r\nConnection: close\r\n\r\n";

Stream& GithubClient::stream(const String& path) {
  if (!client.connect(domain.c_str(), 443)) {
    Serial.println(F("Failed to connect to github over HTTPS."));
    return client;
  }

  char buffer[strlen_P(HTTP_REQUEST_FORMAT)+1];
  strcpy_P(buffer, HTTP_REQUEST_FORMAT);

  client.printf(
    buffer,
    path.c_str(),
    domain.c_str()
  );

  return client;
}

bool GithubClient::download(const String& path, Stream& dest) {
  Stream& client = stream(path);

  if (client.available()) {
    if (!client.find("\r\n\r\n")) {
      Serial.println(F("Error seeking to body"));
      return false;
    }
  } else {
    Serial.println(F("Failed to open stream to Github"));
    return false;
  }

  Serial.println(F("Downloading..."));

  size_t bytes = 0;
  size_t nextCheckpoint = 4096;

  while (client.available()) {
    size_t l = client.readBytes(buffer, GITHUB_CLIENT_BUFFER_SIZE);
    size_t w = dest.write(buffer, l);

    dest.flush();

    if (w != l) {
      printf_P(PSTR("Error writing to stream. Expected to write %d bytes, but only wrote %d\n"), l, w);
      return false;
    }

    bytes += w;

    if (bytes % 10 == 0) {
      printf_P(".");
    }

    if (bytes >= nextCheckpoint) {
      printf("[%d KB]\n", bytes/1024);
      nextCheckpoint += 4096;
    }

    yield();
  }

  Serial.println(F("\n"));

  return true;
}

bool GithubClient::download(const String& path, const String& fsPath) {
  String tmpFile = fsPath + ".download_tmp";
  File f = SPIFFS.open(tmpFile.c_str(), "w");

  if (!f) {
    Serial.print(F("ERROR - could not open file for downloading: "));
    Serial.println(fsPath);
    return false;
  }
  printf(".");

  if (!download(path, f)) {
    f.close();
    return false;
  }

  f.flush();
  f.close();

  SPIFFS.remove(fsPath);
  SPIFFS.rename(tmpFile, fsPath);

  printf_P(PSTR("Finished downloading file: %s\n"), fsPath.c_str());

  return true;
}

String GithubClient::buildRepoPath(const String& username, const String& repo, const String& repoPath) {
  String path = String("/") + username + "/" + repo + "/master/" + repoPath;
  return path;
}

String GithubClient::buildApiRequest(const String &username, const String &repo, const String &path) {
  return String("/repos/") + username + "/" + repo + path;
}

GithubClient GithubClient::rawDownloader() {
  return GithubClient(GITHUB_RAW_DOMAIN);
}

GithubClient GithubClient::apiClient() {
  return GithubClient(GITHUB_API_DOMAIN);
}
