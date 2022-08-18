#define __NVS_MGR__ 1
#include "nvs_mgr.hpp"

bool
NVSMgr::setup(bool force_erase)
{
  bool erased = false;
  esp_err_t err;

  initialized = false;
  track_count = 0;
  next_idx    = 0;
  
  track_list.clear();

  if (force_erase) {
    if ((err = nvs_flash_erase()) == ESP_OK) {
      err    = nvs_flash_init();
      erased = true;
    }
  }
  else {
    err = nvs_flash_init();
    if (err != ESP_OK) {
      if ((err == ESP_ERR_NVS_NO_FREE_PAGES) || (err == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
        ESP_LOGI(TAG, "Erasing NVS Partition... (Because of %s)", esp_err_to_name(err));
        if ((err = nvs_flash_erase()) == ESP_OK) {
          err    = nvs_flash_init();
          erased = true;
        }
      }
    }
  } 

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "NVS Error: %s", esp_err_to_name(err));
  }
  else {
    initialized = true;
  }

  return initialized;
}

bool 
NVSMgr::get(char * segment_name, uint8_t * data, size_t size) 
{
  nvs_handle_t nvs_handle;
  esp_err_t err;
  size_t length = size;
  bool result = false;

  if ((err = nvs_open(segment_name, NVS_READONLY, &nvs_handle)) == ESP_OK) {
    if ((err = nvs_get_blob(nvs_handle, segment_name, data, &length)) == ESP_OK) {
      result = length == size;
    }
    else {
      ERR_LOGE(TAG, "Unable to read NVS data: %s.", esp_err_to_name(err));
    }
    nvs_close(nvs_handle);
  }
  else {
    ERR_LOGE(TAG, "Unable to open NVS segment %s: %s.", segment_name, esp_err_to_name(err));
  }

  return result;
}

bool 
NVSMgr::put(char * segment_name, uint8_t * data, size_t size) 
{
  nvs_handle_t nvs_handle;
  esp_err_t err;
  bool result = false;

  if ((err = nvs_open(segment_name, NVS_READWRITE, &nvs_handle)) == ESP_OK) {
    if ((err = nvs_set_blob(nvs_handle, segment_name, data, size)) == ESP_OK) {
      result = true;
    }
    else {
      ERR_LOGE(TAG, "Unable to write NVS data: %s.", esp_err_to_name(err));
    }
    nvs_close(nvs_handle)
  }
  else {
    ERR_LOGE(TAG, "Unable to open NVS segment %s: %s.", segment_name, esp_err_to_name(err));
  }

  return result;
}