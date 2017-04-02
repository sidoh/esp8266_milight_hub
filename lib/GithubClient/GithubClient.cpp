#include <GithubClient.h>
#include <FS.h>

Stream& GithubClient::stream(const String& path) {
  if (!client.connect(GITHUB_RAW_DOMAIN, 443)) {
    Serial.println(F("Failed to connect to github over HTTPS."));
  }
  
  if (!client.verify(sslFingerprint.c_str(), domain.c_str())) {
    Serial.println(F("Failed to verify github certificate"));
  }
  
  client.printf(
    "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
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
  
  printf("1.");
  
  if (!f) {
    Serial.print(F("ERROR - could not open file for downloading: "));
    Serial.println(fsPath);
    return false;
  }
  printf("2.");
  
  if (!download(path, f)) {
    f.close();
    return false;
  }
  printf("3.");
  
  f.flush();
  f.close();
  
  SPIFFS.remove(fsPath);
  SPIFFS.rename(tmpFile, fsPath);
  
  printf("Finished downloading file: %s\n", fsPath.c_str());
  
  return true;
}
  
String GithubClient::buildRepoPath(const String& username, const String& repo, const String& repoPath) {
  String path = String("/") + username + "/" + repo + "/master/" + repoPath;
  return path;
}
  
GithubClient GithubClient::rawDownloader() {
  return GithubClient(GITHUB_RAW_DOMAIN, GITHUB_RAW_FINGERPRINT);
}

GithubClient GithubClient::apiClient() {
  return GithubClient(GITHUB_API_DOMAIN, GITHUB_API_FINGERPRINT);
}