// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#ifndef LOG_LOCAL_LEVEL
  #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#endif

#include "esp_log.h"

// Shorter versions of the standard LOG functions. 

#define LOG_E(fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...) ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
#define LOG_D(fmt, ...) ESP_LOGD(TAG, fmt, ##__VA_ARGS__)