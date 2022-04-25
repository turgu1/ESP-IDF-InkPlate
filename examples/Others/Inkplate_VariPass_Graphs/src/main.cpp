/*
   Inkplate_VariPass_Graphs example for e-radionica Inkplate6
   For this example you will need a micro USB cable, Inkplate6, and an available WiFi connection.
   Select "Inkplate 6(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 6(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/

   This example will show you how you can use the API on the VariPass website to download and display
   a sensor graph on the e-paper display.

   VariPass is a website which allows you to host various online "variables" which you can write to
   and read from using the VariPass API. This allows you to store sensor logs and later retrieve them
   for graphing, analysis, etc.
   This example uses an already public variable as an example. The graph is fed every minute with data
   from Thorinair's (https://github.com/Thorinair/) geiger counter, so each startup of the Inkplate will
   display updated values.

   To learn more about VariPass and how you can use it for your own projects, please visit: https://varipass.org/
   If you want to easily integrate the read/write functionality in your project, use the official library:
   https://github.com/Thorinair/VariPass-for-ESP8266-ESP32

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   23 July 2020 by Soldered
*/

// If your Inkplate doesn't have external (or second) MCP I/O expander, you should uncomment next line,
// otherwise your code could hang out when you send code to your Inkplate.
// You can easily check if your Inkplate has second MCP by turning it over and 
// if there is missing chip near place where "MCP23017-2" is written, but if there is
// chip soldered, you don't have to uncomment line and use external MCP I/O expander
//#define ONE_MCP_MODE

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "secur.hpp"

#include "inkplate.hpp"            //Include Inkplate library to the sketch
Inkplate display(DisplayMode::INKPLATE_1BIT); // Create an object on Inkplate library and also set library into 1 Bit mode (BW)

static const char * TAG = "VariPassGraph";

const char * ssid     = YOUR_SSID;      // Your WiFi SSID
const char * password = YOUR_PASSWORD;  // Your WiFi password

void delay(int msec) { vTaskDelay(msec / portTICK_PERIOD_MS); }

void mainTask(void * param)
{
  int w, h;

  display.begin();        // Init Inkplate library (you should call this function ONLY ONCE)
  display.clearDisplay(); // Clear frame buffer of display
  display.display();      // Put clear image on display

  w = display.width();
  h = display.height();

  display.print("Connecting to WiFi...");
  display.partialUpdate();

  // Connect to the WiFi network.
  if (display.joinAP(ssid, password)) {

    ESP_LOGI(TAG, "Connected. Downloading the image...");

    display.println("\nWiFi OK! Downloading...");
    display.partialUpdate();

    // Use a HTTP get request to fetch the graph from VariPass.
    // The API expects a few parameters in the URL to allow it to work.
    //  action - Should be set to "sgraph" or "graph" in order to generate a compatible image.
    //  id     - ID of the variable. It is enough to specify just the ID if the variable is public,
    //           but a "key" parameter should also be specified if not.
    //  width  - Width of the generated graph, here set to half the Inkplate's width.
    //  height - Height of the generated graph, here set to half the Inkplate's height.
    //  eink   - Should be set to true to generate a BW 1 bit bitmap better suitable for Inkplate.
    // For more detailed explanation and more parameters, please visit the docs page: https://varipass.org/docs/
    
    if (!display.drawBitmapFromWeb("https://api.varipass.org/?action=sgraph&id=kbg3eQfA&width=400&height=300&eink=true",
                                   (w / 2) - 200, (h / 2) - 150))
    {
      ESP_LOGE(TAG, "Not able to retrieve and draw image from web address.");
      display.println("Image open error");
    }
    display.partialUpdate();

    display.disconnect();
  }

  for (;;) {
    ESP_LOGI(TAG, "Completed...");
    delay(10000);
  }
}


#define STACK_SIZE 10000

extern "C" {

  void app_main()
  {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    TaskHandle_t xHandle = NULL;

    xTaskCreate(mainTask, "mainTask", STACK_SIZE, (void *) 1, tskIDLE_PRIORITY, &xHandle);
    configASSERT(xHandle);
  }

} // extern "C"
