// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#pragma once

#ifndef LOG_LOCAL_LEVEL
  #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#endif

#if 0
  #include "esp_log.h"

  #include <format>

// Shorter versions of the standard LOG functions using C++ std::format()
// formatting standard instead of printf() formatting.

  #define LOG_E(fmt, ...) ESP_LOGE(TAG, "%s", std::format(fmt, ## __VA_ARGS__).c_str())
  #define LOG_I(fmt, ...) ESP_LOGI(TAG, "%s", std::format(fmt, ## __VA_ARGS__).c_str())
  #define LOG_D(fmt, ...) ESP_LOGD(TAG, "%s", std::format(fmt, ## __VA_ARGS__).c_str())
  #define LOG_V(fmt, ...) ESP_LOGV(TAG, "%s", std::format(fmt, ## __VA_ARGS__).c_str())
  #define LOG_W(fmt, ...) ESP_LOGW(TAG, "%s", std::format(fmt, ## __VA_ARGS__).c_str())

#endif

#include <format>
#include <utility>

#include "esp_log.h"

namespace logging {

  enum class Level { Debug, Info, Warn, Error, Verbose };

  template<Level L, typename ... Args>
  void log(const char* tag, std::format_string<Args...> fmt, Args&&... args) {
    // Format the string safely using C++20 std::format
    auto formatted = std::format(fmt, std::forward<Args>(args)...);

    // Dispatch to the correct ESP-IDF macro using "%s" to pass the raw string safely
    if constexpr (L == Level::Debug) { ESP_LOGD(tag, "%s", formatted.c_str()); }
    else if constexpr (L == Level::Info) { ESP_LOGI(tag, "%s", formatted.c_str()); }
    else if constexpr (L == Level::Warn) { ESP_LOGW(tag, "%s", formatted.c_str()); }
    else if constexpr (L == Level::Error) { ESP_LOGE(tag, "%s", formatted.c_str()); }
    else if constexpr (L == Level::Verbose) { ESP_LOGV(tag, "%s", formatted.c_str()); }
  }

} // namespace esp_log_cpp

// Override/Define your custom clean macros
// The ##__VA_ARGS__ handles zero-argument formats cleanly

#define LOG_D(fmt, ...) ::logging::log<::logging::Level::Debug>(TAG, fmt, ## __VA_ARGS__)
#define LOG_I(fmt, ...) ::logging::log<::logging::Level::Info>(TAG, fmt, ## __VA_ARGS__)
#define LOG_W(fmt, ...) ::logging::log<::logging::Level::Warn>(TAG, fmt, ## __VA_ARGS__)
#define LOG_E(fmt, ...) ::logging::log<::logging::Level::Error>(TAG, fmt, ## __VA_ARGS__)
#define LOG_V(fmt, ...) ::logging::log<::logging::Level::Verbose>(TAG, fmt, ## __VA_ARGS__)
