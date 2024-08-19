// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#pragma once

#include <cinttypes>

#include "non_copyable.hpp"
#include "driver/i2c_master.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class Wire : NonCopyable
{
  private:
    static constexpr char const * TAG = "Wire";

    static SemaphoreHandle_t mutex;
    static StaticSemaphore_t mutex_buffer;
    static const uint8_t BUFFER_LENGTH = 30;
    
    bool    initialized;
    static  Wire singleton;

    uint8_t buffer[BUFFER_LENGTH];
    uint8_t address;
    uint8_t size_to_read;
    uint8_t index;

    Wire() : 
     initialized(false), 
      size_to_read(0),
      index(0)
      {}
    
    i2c_cmd_handle_t cmd;

  public:
    static inline Wire & get_singleton() noexcept { return singleton; }

    void       setup();
    void       begin_transmission(uint8_t addr);
    esp_err_t  end_transmission();
    void       write(uint8_t val);
    uint8_t    read();
    esp_err_t  request_from(uint8_t addr, uint8_t size);

    inline static void enter() { xSemaphoreTake(mutex, portMAX_DELAY); }
    inline static void leave() { xSemaphoreGive(mutex); }
};

#if __WIRE__
  Wire & wire = Wire::get_singleton();
#else
  extern Wire & wire;
#endif
