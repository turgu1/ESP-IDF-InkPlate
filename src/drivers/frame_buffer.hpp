#pragma once

#include "esp_log.h"

#include <cstdint>
#include <cstring>

class FrameBuffer 
{
  protected:
    static constexpr char const * TAG = "FrameBuffer";
    const int32_t data_size;
    const int16_t width, height, line_size;
    const uint8_t init_value;

  public:
    FrameBuffer(int16_t w, int16_t h, int32_t s, uint8_t i) : 
      data_size(s), width(w), height(h), line_size(s / h), init_value(i) {}

    inline int16_t       get_width() { return width;      }
    inline int16_t      get_height() { return height;     }
    inline int32_t   get_data_size() { return data_size;  }
    inline int16_t   get_line_size() { return line_size;  }
    inline uint8_t  get_init_value() { return init_value; }
    
    void              clear() {
      // ESP_LOGD(TAG, "Clear: %08x, with: %02x, size: %d", (int)get_data(), (int)get_init_value(), (int)get_data_size());
      memset(get_data(), get_init_value(), get_data_size()); 
    } 

    virtual uint8_t * get_data() = 0;
};

class FrameBuffer1Bit : public FrameBuffer 
{
  public:
    FrameBuffer1Bit(int16_t w, int16_t h, int32_t s) : FrameBuffer(w, h, s, 0) {}
};

class FrameBuffer3Bit : public FrameBuffer 
{
  public:
    FrameBuffer3Bit(int16_t w, int16_t h, int32_t s) : FrameBuffer(w, h, s, (uint8_t) 0x77) {}
};
