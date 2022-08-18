#pragma once

#include "nvs_flash.h"

class NVSMgr
{
  public:
    NVSMgr() : initialized(false) {}
   ~NVSMgr() {}

    bool setup(bool force_erase = false);
    bool get(char * segment_name, uint8_t * buffer, uint16_t size);

  private:
    static constexpr char const * TAG            = "NVSMgr";
    static constexpr char const * PARTITION_NAME = "nvs";

    nvs_handle_t nvs_handle;
    bool         initialized;
};
