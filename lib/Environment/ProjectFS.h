//
// Created by chris on 9/14/2023.
//

#ifndef ESP8266_MILIGHT_HUB_PROJECTFS_H
#define ESP8266_MILIGHT_HUB_PROJECTFS_H

#ifdef MILIGHT_USE_LITTLE_FS
#include <LittleFS.h>
#define ProjectFS LittleFS
#else
#include <FS.h>
#define ProjectFS SPIFFS
#endif

#endif //ESP8266_MILIGHT_HUB_PROJECTFS_H
