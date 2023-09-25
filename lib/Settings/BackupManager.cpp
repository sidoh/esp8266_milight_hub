//
// Created by chris on 9/18/2023.
//

#include <BackupManager.h>
#include <ProjectFS.h>
#include <StreamUtils.h>

const uint8_t BackupManager::SETTINGS_BACKUP_VERSION = 1;
const uint32_t BackupManager::SETTINGS_MAGIC_HEADER = 0x92A7C300 | SETTINGS_BACKUP_VERSION;

void BackupManager::createBackup(const Settings& settings, Stream& stream) {
  stream.write(reinterpret_cast<const char*>(&SETTINGS_MAGIC_HEADER), sizeof(SETTINGS_MAGIC_HEADER));

  GroupAlias::saveAliases(stream, settings.groupIdAliases);
  settings.serialize(stream);
}

BackupManager::RestoreStatus BackupManager::restoreBackup(Settings& settings, Stream& stream) {
  uint32_t magicHeader = 0;
  // read the header
  stream.readBytes(reinterpret_cast<char*>(&magicHeader), sizeof(magicHeader));

  // Check the header
  if ((magicHeader & 0xFFFFFF00) != (SETTINGS_MAGIC_HEADER & 0xFFFFFF00)) {
    Serial.printf_P(PSTR("ERROR: invalid backup file header. expected %08X but got %08X\n"), SETTINGS_MAGIC_HEADER & 0xFFFFFF00, magicHeader & 0xFFFFFF00);
    return BackupManager::RestoreStatus::INVALID_HEADER;
  }

  // Check the version
  if ((magicHeader & 0xFF) != SETTINGS_BACKUP_VERSION) {
    Serial.printf_P(PSTR("ERROR: invalid settings file version. expected %d but got %d\n"), SETTINGS_BACKUP_VERSION, magicHeader & 0xFF);
    return BackupManager::RestoreStatus::INVALID_VERSION;
  }

  // reset settings to default
  settings = Settings();

  Serial.printf_P(PSTR("Restoring %d byte backup\n"), stream.available());
  GroupAlias::loadAliases(stream, settings.groupIdAliases);

  // read null terminator
  stream.read();

  // Save to persist aliases
  settings.save();

  // Copy remaining part of the buffer to the settings file

  File f = ProjectFS.open(SETTINGS_FILE, "w");
  Serial.println(F("Restoring settings file"));
  if (!f) {
    Serial.println(F("Opening settings file failed"));
    return BackupManager::RestoreStatus::INVALID_FILE;
  } else {
    Serial.printf_P(PSTR("%d bytes remaining in backup\n"), stream.available());
    WriteBufferingStream bufferedStream(f, 128);

    while (stream.available()) {
      bufferedStream.write(stream.read());
    }

    bufferedStream.flush();
    f.close();
  }

  // Reload settings
  Settings::load(settings);
  settings.save();

  return BackupManager::RestoreStatus::OK;
}