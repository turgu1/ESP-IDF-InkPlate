// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#pragma once

#define DEBUG_WIRE 0

#include <cinttypes>
#include <cstring>

#include "non_copyable.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver/i2c_master.h"

class Wire : NonCopyable
{
  private:
    static constexpr char const * TAG = "Wire";

    static SemaphoreHandle_t mutex;
    static StaticSemaphore_t mutex_buffer;

    i2c_master_bus_handle_t master_bus_handle;
    
    bool    initialized;
    static  Wire singleton;

    Wire() : initialized(false) {} 

  public:
    static inline Wire & get_singleton() noexcept { return singleton; }

    void       setup();

    inline static void enter() { xSemaphoreTake(mutex, portMAX_DELAY); }
    inline static void leave() { xSemaphoreGive(mutex); }

    inline void init_device(uint8_t addr, i2c_master_dev_handle_t * dev_handle) {
      i2c_device_config_t dev_cfg;

      dev_cfg.dev_addr_length         = I2C_ADDR_BIT_LEN_7;
      dev_cfg.device_address          = addr;
      dev_cfg.scl_speed_hz            = 1E5;
      dev_cfg.scl_wait_us             = 500;
      dev_cfg.flags.disable_ack_check = false;

      ESP_ERROR_CHECK(i2c_master_bus_add_device(master_bus_handle, &dev_cfg, dev_handle));
    }

    inline bool sense(uint8_t address) {

      return i2c_master_probe(master_bus_handle, address, 10) == ESP_OK;
    }
    
    inline bool flush() {
      return i2c_master_bus_wait_all_done(master_bus_handle, -1) == ESP_OK;
    }    
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
    uint8_t address;

    i2c_master_dev_handle_t dev_handle;
    
  public:
    WireDevice(uint8_t addr, bool sensing = false) {
      wire.setup();
      if (!sensing || wire.sense(addr)) {
        address = addr;
        wire.init_device(addr, &dev_handle);
        initialized = true;
      }
    }

    inline bool is_initialized() { return initialized; }

    #if DEBUG_WIRE
      void show_data(const char *from, const uint8_t *data, int length, const uint8_t *cmd = nullptr, int cmd_length = 0) {
        printf("----- %s (%02" PRIx8 ") length: %d: -----\n", from, address, length);
        if (cmd != nullptr) {
          printf("cmd = ");
          for (int i = 0; i < cmd_length; i++) {
            printf("%02x ", cmd[i]);
          }
          printf("\n");
        }
        if (data != nullptr) {
          printf("data = ");
          for (int i = 0; i < length; i++) {
            printf("%02x ", data[i]);
          }
          printf("\n");
        }
        fflush(stdout);
      }
    #endif

    inline bool write(const uint8_t *data, int length, int timeout = 500) {
      #if DEBUG_WIRE
        show_data("write", data, length);
      #endif

      return i2c_master_transmit(dev_handle, data, length, timeout) == ESP_OK;
    }

    bool cmd_write(uint8_t cmd, const uint8_t *data, int length, int timeout = 500) {
      bool result;

      uint8_t *buff = new uint8_t[length + 1];

      if (buff == nullptr) return false;

      buff[0] = cmd;
      std::memcpy(&buff[1], data, length);

      result = write(buff, length + 1, timeout);

      delete [] buff;

      return result;
    }

    inline bool cmd_write(uint8_t cmd, int timeout = 500) {
      #if DEBUG_WIRE
        show_data("cmd_write", nullptr, 0, &cmd, 1);
      #endif
      return i2c_master_transmit(dev_handle, &cmd, 1, timeout) == ESP_OK;
    }

    inline bool cmd_write(uint8_t cmd, uint8_t data, int timeout = 500) {
      bool result;

      uint8_t buff[2] = { cmd, data };

      #if DEBUG_WIRE
        show_data("cmd_write", &data, 1, &cmd, 1);
      #endif
      result = i2c_master_transmit(dev_handle, buff, 2, timeout) == ESP_OK;

      return result;
    }

    inline bool read(uint8_t *data, int length, int timeout = 500) {

      if (length == 0) return false;

      #if DEBUG_WIRE
        printf("----- read (%02" PRIx8 ") length: %d -----\n", address, length);
      #endif


      esp_err_t status = i2c_master_receive(dev_handle, data, length, timeout);
      bool result = status == ESP_OK;
      #if DEBUG_WIRE
        if (result) {
          show_data("got", data, length);
        } else {
          ESP_LOGW(TAG, "No answer for device %02" PRIx8 ": error: %s", address, esp_err_to_name(status));
        }
      #endif

      return result;
    }

    inline bool cmd_read(uint8_t cmd, uint8_t *data, int length, int timeout = 500) {

      #if DEBUG_WIRE
        printf("----- cmd_read (%02" PRIx8 ") cmd: %02" PRIi8 " length: %d -----\n", address, cmd, length);
      #endif

      esp_err_t status = i2c_master_transmit_receive(dev_handle, &cmd, 1, data, length, timeout);
      bool result = status == ESP_OK;
      #if DEBUG_WIRE
        if (result) {
          show_data("got", data, length);
        } else {
          ESP_LOGW(TAG, "No answer for device %02" PRIx8 ": error: %s", address, esp_err_to_name(status));
        }
      #endif
      return result;
    }  

    inline uint8_t cmd_read(uint8_t cmd, int timeout = 500) {
      uint8_t data = 0;

      #if DEBUG_WIRE
        printf("----- cmd_read (%02" PRIx8 ") cmd: %02" PRIi8 " length: 1 -----\n", address, cmd);
      #endif
      
      #if DEBUG_WIRE
        esp_err_t status = i2c_master_transmit_receive(dev_handle, &cmd, 1, &data, 1, timeout);
        bool result = status == ESP_OK;
        if (result) {
          show_data("got", &data, 1);
        } else {
          ESP_LOGW(TAG, "No answer for device %02" PRIx8 ": error: %s", address, esp_err_to_name(status));
        }
      #else
        i2c_master_transmit_receive(dev_handle, &cmd, 1, &data, 1, 500);
      #endif

      return data;
    }  

    inline bool cmd_read(const uint8_t *cmd, int cmd_length, uint8_t *data, int length) {

      #if DEBUG_WIRE
        show_data("cmd_read", nullptr, 0, cmd, cmd_length);
      #endif

      esp_err_t status = i2c_master_transmit_receive(dev_handle, cmd, cmd_length, data, length, 500);
      bool result = status == ESP_OK;
      #if DEBUG_WIRE
        if (result) {
          show_data("got", data, length);
        } else {
          ESP_LOGW(TAG, "No answer for device %02" PRIx8 ": error: %s", address, esp_err_to_name(status));
        }
      #endif
      return result;
    }
};