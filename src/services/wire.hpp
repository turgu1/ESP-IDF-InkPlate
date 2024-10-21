// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#pragma once

// The following defines are used in the context of migrating from the I2C legacy
// interface provided with ESP-IDF to the new I2C_MASTER interface.
//
// In the same vein, it was decided to merge into a new WireDevice class all specific 
// calls to the I2C component, eliminating the nitty-gritty calls from anywere else.
//
// The migration is done in 3 phases:
//
// 1. Change all occurences of direct calls to the Wire class to use the WebDevice class
//    keeping the legacy-based calls (I2C_LEGACY_BASED set to 1). Can be done one code segement
//    at a time by setting TRANSITION_TO_NEW_API_COMPLETED to 0.
// 2. Once tested and all occurences translated to the new WebDevice class, set
//    the TRANSITION_TO_NEW_API_COMPLETED to 1. Do some testing.
//
// 3. Set the I2C_LEGACY_BASED to 0. It will then use the new I2C_MASTER interface.

#define I2C_LEGACY_BASED 0

#if I2C_LEGACY_BASED
  #define TRANSITION_TO_NEW_API_COMPLETED 1
  #define I2C_MASTER_BASED 0
#else
  #define I2C_MASTER_BASED 1
#endif

#define DEBUG_WIRE 0

#include <cinttypes>
#include <cstring>

#include "non_copyable.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_err.h"

#if I2C_LEGACY_BASED
  #include "driver/i2c.h"
#else
  #include "driver/i2c_master.h"
#endif

class Wire : NonCopyable
{
  private:
    static constexpr char const * TAG = "Wire";

    static SemaphoreHandle_t mutex;
    static StaticSemaphore_t mutex_buffer;

    #if I2C_LEGACY_BASED
      static const uint8_t BUFFER_LENGTH = 30;
      uint8_t buffer[BUFFER_LENGTH];
      uint8_t address;
      uint8_t size_to_read;
      uint8_t index;
    #else
      i2c_master_bus_handle_t master_bus_handle;
    #endif
    
    bool    initialized;
    static  Wire singleton;

    Wire() : initialized(false) {} 

  public:
    static inline Wire & get_singleton() noexcept { return singleton; }

    void       setup();

    #if I2C_LEGACY_BASED && !TRANSITION_TO_NEW_API_COMPLETED

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
      
      void       begin_transmission(uint8_t addr);
      esp_err_t  end_transmission();
      void       write_byte(uint8_t val);
      uint8_t    read_byte();
      esp_err_t  request_from(uint8_t addr, uint8_t size);
    #endif

    inline static void enter() { xSemaphoreTake(mutex, portMAX_DELAY); }
    inline static void leave() { xSemaphoreGive(mutex); }

    #if I2C_MASTER_BASED
      inline void init_device(uint8_t addr, i2c_master_dev_handle_t * dev_handle) {
        i2c_device_config_t dev_cfg;

        dev_cfg.dev_addr_length         = I2C_ADDR_BIT_LEN_7;
        dev_cfg.device_address          = addr;
        dev_cfg.scl_speed_hz            = 1E5;
        dev_cfg.scl_wait_us             = 500000;
        dev_cfg.flags.disable_ack_check = false;

        ESP_ERROR_CHECK(i2c_master_bus_add_device(master_bus_handle, &dev_cfg, dev_handle));
      }
    #endif

    inline bool sense(uint8_t address) {
      #if I2C_LEGACY_BASED
        // ToDo add some delay
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        bool result = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100000 / portTICK_PERIOD_MS) == ESP_OK;
        i2c_cmd_link_delete(cmd);

        return result;
      #endif

      #if I2C_MASTER_BASED
        return i2c_master_probe(master_bus_handle, address, 10) == ESP_OK;
      #endif
    }
    
    inline bool flush() {
      #if I2C_MASTER_BASED
        return i2c_master_bus_wait_all_done(master_bus_handle, -1) == ESP_OK;
      #else
        return true;
      #endif
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

    #if I2C_MASTER_BASED
      i2c_master_dev_handle_t dev_handle;
    #endif
    
  public:
    WireDevice(uint8_t addr, bool sensing = false) {
      wire.setup();
      if (!sensing || wire.sense(addr)) {
        address = addr;
        #if I2C_MASTER_BASED
          wire.init_device(addr, &dev_handle);
        #endif
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

      #if I2C_LEGACY_BASED
        bool result = false;

        if (initialized) {
          //ESP_LOGD(TAG, "Writing %d bytes to i2c at address 0x%02x.", index, address);
          //   printf("%02x ", buffer[i]);
          // }
          // printf("\n");
          // fflush(stdout);


          i2c_cmd_handle_t cmd = i2c_cmd_link_create();
          ESP_ERROR_CHECK(i2c_master_start(cmd));
          ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1));
          if (length > 0) ESP_ERROR_CHECK(i2c_master_write(cmd, data, length, 1));
          ESP_ERROR_CHECK(i2c_master_stop(cmd));
          esp_err_t status = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
          result = status == ESP_OK;
          i2c_cmd_link_delete(cmd);

          if (!result) {
            ESP_LOGE(TAG, "Unable to complete the write action (%02" PRIx8 "): %s.", address, esp_err_to_name(status));
          }
        }

        return result;
      #else
        return i2c_master_transmit(dev_handle, data, length, timeout) == ESP_OK;
      #endif
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
      #if I2C_LEGACY_BASED
        return write(&cmd, 1);
      #else
        #if DEBUG_WIRE
          show_data("cmd_write", nullptr, 0, &cmd, 1);
        #endif
        return i2c_master_transmit(dev_handle, &cmd, 1, timeout) == ESP_OK;
      #endif
    }

    inline bool cmd_write(uint8_t cmd, uint8_t data, int timeout = 500) {
      bool result;

      uint8_t buff[2] = { cmd, data };

      #if I2C_LEGACY_BASED
        return write(buff, 2);
      #else
        #if DEBUG_WIRE
          show_data("cmd_write", &data, 1, &cmd, 1);
        #endif
        result = i2c_master_transmit(dev_handle, buff, 2, timeout) == ESP_OK;
      #endif

      return result;
    }

    inline bool read(uint8_t *data, int length, int timeout = 500) {

      if (length == 0) return false;

      #if DEBUG_WIRE
        printf("----- read (%02" PRIx8 ") length: %d -----\n", address, length);
      #endif

      #if I2C_LEGACY_BASED
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, true));

        if (length > 1) ESP_ERROR_CHECK(i2c_master_read(cmd, data, length - 1, I2C_MASTER_ACK));
        ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data + length - 1, I2C_MASTER_LAST_NACK));

        ESP_ERROR_CHECK(i2c_master_stop(cmd));

        esp_err_t status = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        bool result = status == ESP_OK;

        if (!result) {
          ESP_LOGE(TAG, "Unable to complete the read action(%02" PRIx8 "): %s.", address, esp_err_to_name(status));
        }
        #if DEBUG_WIRE
          else {
            show_data("got", data, length);
          }
        #endif
        return result;
      #else
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
      #endif
    }

    inline bool cmd_read(uint8_t cmd, uint8_t *data, int length, int timeout = 500) {

      #if DEBUG_WIRE
        printf("----- cmd_read (%02" PRIx8 ") cmd: %02" PRIi8 " length: %d -----\n", address, cmd, length);
      #endif

      #if I2C_LEGACY_BASED
        if (write(&cmd, 1)) {
          return read(data, length);
        } else {
          return false;
        }
      #else
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
      #endif
    }  

    inline uint8_t cmd_read(uint8_t cmd, int timeout = 500) {
      uint8_t data = 0;

      #if DEBUG_WIRE
        printf("----- cmd_read (%02" PRIx8 ") cmd: %02" PRIi8 " length: 1 -----\n", address, cmd);
      #endif
      
      #if I2C_LEGACY_BASED
        if (write(&cmd, 1)) {
          read(&data, 1);
        }
      #else
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
      #endif

      return data;
    }  

    inline bool cmd_read(const uint8_t *cmd, int cmd_length, uint8_t *data, int length) {

      #if DEBUG_WIRE
        show_data("cmd_read", nullptr, 0, cmd, cmd_length);
      #endif

      #if I2C_LEGACY_BASED
        if (write(cmd, cmd_length)) {
          return read(data, length);
        } else {
          return false;
        }
      #else
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
      #endif
    }
};