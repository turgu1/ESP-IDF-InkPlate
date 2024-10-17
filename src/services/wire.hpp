// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#pragma once

#include <cinttypes>
#include <cstring>

#include "non_copyable.hpp"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

class Wire : NonCopyable
{
  private:
    static constexpr char const * TAG = "Wire";

    static SemaphoreHandle_t mutex;
    static StaticSemaphore_t mutex_buffer;
    // static const uint8_t BUFFER_LENGTH = 30;
    
    bool    initialized;
    static  Wire singleton;

    // uint8_t buffer[BUFFER_LENGTH];
    // uint8_t address;
    // uint8_t size_to_read;
    // uint8_t index;

    Wire() : initialized(false) {}
    
    i2c_master_bus_handle_t master_bus_handle;

  public:
    static inline Wire & get_singleton() noexcept { return singleton; }

    void       setup();
    // void       begin_transmission(uint8_t addr);
    // esp_err_t  end_transmission();
    // void       write(uint8_t val);
    // uint8_t    read();
    // esp_err_t  request_from(uint8_t addr, uint8_t size);

    inline static void enter() { xSemaphoreTake(mutex, portMAX_DELAY); }
    inline static void leave() { xSemaphoreGive(mutex); }

    inline void init_device(uint8_t addr, i2c_master_dev_handle_t *dev_handle) {
      i2c_device_config_t dev_cfg;

      dev_cfg.dev_addr_length   = I2C_ADDR_BIT_LEN_7;
      dev_cfg.device_address    = addr;
      dev_cfg.scl_speed_hz      = 1E5;
      dev_cfg.scl_wait_us       = 500000;
      dev_cfg.flags.disable_ack_check = false;

      ESP_ERROR_CHECK(i2c_master_bus_add_device(master_bus_handle, &dev_cfg, dev_handle));
    }

    inline bool sense(uint8_t addr) {
      return i2c_master_probe(master_bus_handle, addr, 500) == ESP_OK;
    }

    // void check_asynch() {
    //   ESP_LOGI(TAG, "Asynch mode is = %s", master_bus_handle->async_trans ? "true" : "false");
    // }
};

#if __WIRE__
  Wire & wire = Wire::get_singleton();
#else
  extern Wire & wire;
#endif

class WireDevice {
  private:
    static constexpr char const * TAG = "WireDevice";

    bool initialized{false};

    i2c_master_dev_handle_t dev_handle;
    
  public:
    WireDevice(uint8_t addr) {
      wire.setup();
      if (wire.sense(addr)) {
        wire.init_device(addr, &dev_handle);
        initialized = true;
      }
    }

    inline bool is_initialized() { return initialized; }

    inline bool write(const uint8_t *data, int length) {
      return i2c_master_transmit(dev_handle, data, length, 500) == ESP_OK;
    }

    bool cmd_write(uint8_t cmd, const uint8_t *data, int length) {
      uint8_t *buff = new uint8_t[length + 1];

      if (buff == nullptr) return false;
      buff[0] = cmd;
      std::memcpy(&buff[1], data, length);

      bool result = i2c_master_transmit(dev_handle, buff, length + 1, 500) == ESP_OK;

      delete [] buff;

      return result;
    }

    inline bool cmd_write(uint8_t cmd) {
      return i2c_master_transmit(dev_handle, &cmd, 1, 500) == ESP_OK;
    }

    inline bool cmd_write(uint8_t cmd, uint8_t data) {
      uint8_t buff[2] = { cmd, data };

      bool result = i2c_master_transmit(dev_handle, buff, 2, 500) == ESP_OK;

      return result;
    }

    inline bool read(uint8_t *data, int length) {
      return i2c_master_receive(dev_handle, data, length, 500) == ESP_OK;
    }

    inline bool cmd_read(uint8_t cmd, uint8_t *data, int length) {
      return i2c_master_transmit_receive(dev_handle, &cmd, 1, data, length, 500) == ESP_OK;
    }  

    inline uint8_t cmd_read(uint8_t cmd) {
      uint8_t data;
      if (i2c_master_transmit_receive(dev_handle, &cmd, 1, &data, 1, 500) == ESP_OK) {
        return data;
      } else {
        return 0;
      }
    }  

    inline bool cmd_read(const uint8_t *cmd, int cmd_length, uint8_t *data, int length) {
      return i2c_master_transmit_receive(dev_handle, cmd, cmd_length, data, length, 500) == ESP_OK;
    }
};