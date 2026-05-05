#pragma once

#include "esp_idf_version.h"

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(6, 0, 0)

  #include <ctime>

  time_t timegm(struct tm *tm);

#endif
