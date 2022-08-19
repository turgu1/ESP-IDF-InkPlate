#pragma once

#include "nvs_flash.h"

class NVSMgr
{
  private:
    static NVSMgr singleton;

  public:
    NVSMgr() : initialized(false) {}
   ~NVSMgr() {}

    static inline NVSMgr & get_singleton() noexcept { return singleton; }

    bool setup(bool force_erase = false);
    bool get(char * segment_name, uint8_t * data, size_t size); 
    bool put(char * segment_name, uint8_t * data, size_t size); 

  private:
    static constexpr char const * TAG            = "NVSMgr";
    static constexpr char const * PARTITION_NAME = "nvs";

   nvs_handle_t nvs_handle;
    bool         initialized;
};

#if __NVS_MGR__
  NVSMgr & nvs_mgr = NVSMgr::get_singleton();
#else
  extern NVSMgr & nvs_mgr;
#endif
