#pragma once

#include <stdint.h>

#define DEVICE_NAME "es-led-module"
#define FIRMWARE_VERSION "1.0.0"

#define DEVICE_PROJECT "211-command-usb"
#define DEVICE_REPO "https://github.com/takonef/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

void device_info(void); 
void dev_info(void); 

struct info_t
{
    uint32_t version;    // 4 байта первым
    char name[13];       // 13 байт — посередине
    uint8_t revision;    // 1 байт — в хвост
};