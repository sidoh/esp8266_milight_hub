#include <GithubDownloader.h>
#include <FS.h>
  
Stream& GithubDownloader::streamFile(const String& path) {
  if (!client.connect(GITHUB_RAW_DOMAIN, 443)) {
    Serial.println("Failed to connect to github over HTTPS.");
  }
  
  if (!client.verify(GITHUB_SSL_FINGERPRINT, GITHUB_RAW_DOMAIN)) {
    Serial.println("Failed to verify github certificate");
  }
  
  client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + GITHUB_RAW_DOMAIN + "\r\n" +
               "Connection: close\r\n\r\n");
               
  return client;
}
  
Stream& GithubDownloader::streamFile(const String& username, const String& repo, const String& path) {
  return streamFile(buildPath(username, repo, path));
}
  
bool GithubDownloader::downloadFile(const String& path, Stream& dest) {
  Stream& client = streamFile(path);
  
  if (client.available()) {
    if (!client.find("\r\n\r\n")) {
      Serial.println("Error seeking to body");
      return false;
    }
  } else {
    Serial.println("Failed to open stream to Github");
    return false;
  }
               
  while (client.available()) {
    size_t l = client.readBytes(buffer, GITHUB_DOWNLOADER_BUFFER_SIZE);
    size_t w = dest.write(buffer, l);
    
    dest.flush();
    
    if (w != l) {
      printf("Error writing to stream. Expected to write %d bytes, but only wrote %d\n", l, w);
      return false;
    }
    
    printf("Read %d bytes\n", w);
    
    yield();
  }
  
  return true;
}

bool GithubDownloader::downloadFile(const String& username, const String& repo, const String& repoPath, Stream& dest) {
  return downloadFile(buildPath(username, repo, repoPath), dest);
}

bool GithubDownloader::downloadFile(const String& username, const String& repo, const String& repoPath, const String& fsPath) {
  String tmpFile = fsPath + ".download_tmp";
  File f = SPIFFS.open(tmpFile.c_str(), "w");
  
  if (!f) {
    Serial.print("ERROR - could not open file for downloading: ");
    Serial.println(fsPath);
    return false;
  }
  
  if (!downloadFile(buildPath(username, repo, repoPath), f)) {
    f.close();
    return false;
  }
  
  f.flush();
  f.close();
  
  SPIFFS.remove(fsPath);
  SPIFFS.rename(tmpFile, fsPath);
  
  Serial.print("Finished downloading file: ");
  Serial.println(fsPath);
  Serial.println(f.size());
  
  return true;
}
  
String GithubDownloader::buildPath(const String& username, const String& repo, const String& repoPath) {
  String path = String("/") + username + "/" + repo + "/master/" + repoPath;
  return path;
}