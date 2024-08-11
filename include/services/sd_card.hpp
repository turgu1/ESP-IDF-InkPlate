#pragma once

#include <cinttypes>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

class SDCard
{
  public:
    /**
     * @brief SD-Card Setup
     * 
     * This method initialize the ESP-IDF SD-Card capability. This will allow access
     * to the card through standard Posix IO functions or the C++ IOStream.
     * 
     * @return true Initialization Done
     * @return false Some issue
     */
    static bool setup();

  private:
    static constexpr char const * TAG = "SDCard";
    enum class SDCardState : uint8_t { UNINITIALIZED, INITIALIZED, FAILED };
    static SDCardState state;
    static sdmmc_card_t *card;

};
