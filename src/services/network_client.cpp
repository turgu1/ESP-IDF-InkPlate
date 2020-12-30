/*
NetworkClient.cpp
Inkplate 6 Arduino library
David Zovko, Borna Biro, Denis Vajak, Zvonimir Haramustek @ e-radionica.com
September 24, 2020
https://github.com/e-radionicacom/Inkplate-6-Arduino-library

For support, please reach over forums: forum.e-radionica.com/en
For more info about the product, please check: www.inkplate.io

This code is released under the GNU Lesser General Public License v3.0: https://www.gnu.org/licenses/lgpl-3.0.en.html
Please review the LICENSE file included with this example.
If you have any questions about licensing, please contact techsupport@e-radionica.com
Distributed as-is; no warranty is given.
*/

#define __NETWORK_CLIENT__ 1
#include "network_client.hpp"

#include "logging.hpp"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_http_client.h"

static constexpr char const * TAG = "NetworkClient";

static const uint8_t ESP_MAXIMUM_RETRY = 6;

// The event group allows multiple bits for each event, but we 
// only care about two events:
// - we are connected to the AP with an IP
// - we failed to connect after the maximum amount of retries

static const uint8_t WIFI_CONNECTED_BIT = BIT0;
static const uint8_t WIFI_FAIL_BIT      = BIT1;

// FreeRTOS event group to signal when we are connected

static EventGroupHandle_t wifi_event_group;
static bool wifi_first_start = true;

static int s_retry_num = 0;

static void sta_event_handler(void            * arg, 
                              esp_event_base_t  event_base,
                              int32_t           event_id, 
                              void            * event_data)
{
  ESP_LOGI(TAG, "STA Event, Base: %08x, Event: %d.", (unsigned int) event_base, event_id);

  if (event_base == WIFI_EVENT) {
    if (event_id == WIFI_EVENT_STA_START) {
      esp_wifi_connect();
    } 
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
      if (wifi_first_start) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
          vTaskDelay(pdMS_TO_TICKS(10000)); // wait 10 sec
          ESP_LOGI(TAG, "retry to connect to the AP");
          esp_wifi_connect();
          s_retry_num++;
        } 
        else {
          xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
          ESP_LOGI(TAG,"connect to the AP fail");
        }
      }
      else {
        ESP_LOGI(TAG, "Wifi Disconnected.");
        vTaskDelay(pdMS_TO_TICKS(10000)); // wait 10 sec
        ESP_LOGI(TAG, "retry to connect to the AP");
        esp_wifi_connect();
      }
    } 
  }
  else if (event_base == IP_EVENT) {
    if (event_id == IP_EVENT_STA_GOT_IP) {
      ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
      ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
      s_retry_num = 0;
      xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
      wifi_first_start = false;
    }
  }
}

bool 
NetworkClient::joinAP(const char * ssid, const char * pass)
{
  if (connected) disconnect();

  wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                             ESP_EVENT_ANY_ID,
                                             &sta_event_handler,
                                             NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                             IP_EVENT_STA_GOT_IP,
                                             &sta_event_handler,
                                             NULL));
  wifi_config_t wifi_config;

  memset(&wifi_config, 0, sizeof(wifi_config));

  wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
  wifi_config.sta.pmf_cfg.capable    = true;
  wifi_config.sta.pmf_cfg.required   = false;

  strncpy((char *) wifi_config.sta.ssid,     ssid, sizeof(wifi_config.sta.ssid) - 1);
  strncpy((char *) wifi_config.sta.password, pass, sizeof(wifi_config.sta.password) - 1);

  wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = 0;
  wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = 0;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "wifi_init_sta finished.");

  // Waiting until either the connection is established (WIFI_CONNECTED_BIT) 
  // or connection failed for the maximum number of re-tries (WIFI_FAIL_BIT). 
  // The bits are set by event_handler() (see above)

  EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
          WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
          pdFALSE,
          pdFALSE,
          portMAX_DELAY);

  // xEventGroupWaitBits() returns the bits before the call returned, 
  // hence we can test which event actually happened.

  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "connected to ap SSID: %s password: %s", 
      wifi_config.sta.ssid, 
      wifi_config.sta.password);
    connected = true;
  } 
  else if (bits & WIFI_FAIL_BIT) {
    ESP_LOGI(TAG, "Failed to connect to SSID: %s, password: %s", 
      wifi_config.sta.ssid, 
      wifi_config.sta.password);
  }
  else {
    ESP_LOGE(TAG, "UNEXPECTED EVENT");
  }

  if (!connected) {
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
  }
  return connected;
}

void 
NetworkClient::disconnect()
{
  if (connected) {
    // ESP_ERROR_CHECK(esp_event_loop_delete_default());
    
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,   ESP_EVENT_ANY_ID,     &sta_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_START, &sta_event_handler));

    vEventGroupDelete(wifi_event_group);

    esp_wifi_disconnect();

    connected = false;
  }
}

static uint8_t * buffer;
static uint8_t * buffer_ptr;
static int32_t   buffer_size;

static esp_err_t http_event_handler(esp_http_client_event_t * evt)
{
  switch(evt->event_id) {
    case HTTP_EVENT_ERROR:
      ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
      break;
    case HTTP_EVENT_ON_CONNECTED:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
      break;
    case HTTP_EVENT_HEADER_SENT:
      ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
      break;
    case HTTP_EVENT_ON_HEADER:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
      //ESP_LOGI(TAG, "key = %s, value = %s", evt->header_key, evt->header_value);
      if (strcmp("Content-Length", evt->header_key) == 0) {
        buffer_size = atoi(evt->header_value);
        ESP_LOGI(TAG, "Donwload file size: %d", buffer_size);
      }
      break;
    case HTTP_EVENT_ON_DATA:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
      if (!esp_http_client_is_chunked_response(evt->client)) {
        //ESP_LOGI(TAG, "len = %d, %.*s", evt->data_len, evt->data_len, (char*)evt->data);
        if ((buffer == nullptr) && (buffer_size > 0)) {
          buffer_ptr = buffer = (uint8_t *) malloc(buffer_size);
        }
        if (buffer_ptr != nullptr) {
          if ((buffer_ptr + evt->data_len) <= (buffer + buffer_size)) {
            memcpy(buffer_ptr, evt->data, evt->data_len);
            buffer_ptr += evt->data_len;
          }
          else {
            int size = (buffer + buffer_size) - buffer_ptr;
            if ((size > 0) && (size <= evt->data_len)) {
              memcpy(buffer_ptr, evt->data, size);
              buffer_ptr += size;
            }
          }
        }
      }
      break;
    case HTTP_EVENT_ON_FINISH:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
      break;
    case HTTP_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
      break;
  }
  return ESP_OK;
}


uint8_t *
NetworkClient::downloadFile(const char * url, int32_t * defaultLen)
{
  if (!connected) return nullptr;

  ESP_LOGI(TAG, "Downloading file from URL: %s", url);
  
  buffer = buffer_ptr = nullptr;
  buffer_size = *defaultLen;

  esp_http_client_config_t config;

  memset(&config, 0, sizeof(config));
  
  config.url = url;
  config.event_handler = http_event_handler;
  
  esp_http_client_handle_t client = esp_http_client_init(&config);
  esp_err_t err = esp_http_client_perform(client);

  if (err == ESP_OK) {
    ESP_LOGI(TAG, "Status = %d, content_length = %d",
            esp_http_client_get_status_code(client),
            esp_http_client_get_content_length(client));
  }
  esp_http_client_cleanup(client);

  *defaultLen = buffer_size;

  return buffer;
}
