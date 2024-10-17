# ESP-IDF-InkPlate Library Version 0.9.8

(October xxth, 2024)

- Now built using ESP-IDF 5.3.1
- No longer using PlatformIO. Using cmake through idf.py to build applications.
- Added support for Soldered 6PLUS and 6FLICK
- Added support for PCAL GPIO chip (Soldered devices)
- Added support for Cypress touchscreen (6FLICK)
- Added support for SD Card power control (Soldered devices)
- Source code re-org

TBD: How to build programs with the library, examples description

(February 7th, 2023)

A branch named idf-v5.0-support has been created some time ago to permit changes related to ESP-IDF V5.0. Please note that you can try to use it if required. Testing is ongoing to merge it with the upcoming new version.

(June 21st, 2022)

Added support for `idf.py` development, as provided by tajnymag.

(April 27th, 2022)

Version 0.9.6:

- Added MCP presence detection. With chips shortage issues, some Inkplate devices (Inkplate-10, Inkplate-6PLUS) may have their second MCP chip missing. The `mcp_ext.is_present()` method can be used to check its availability. Log information messages are produced indicating if MCPs are detected as present or not.

Version 0.9.5: 

- Corrected issue with PlatformIO Espressif32 version 3.3.0 (wifi_interface_t vs esp_interface_t)
- Added InkPlate-6PLUS support: Screen, Touch Screen, Front Light. `INKPLATE_6PLUS` must be defined to compile for this device. For the TouchScreen class, TouchPositions has been defined as an array of 2 entries to minimize the potential of bad methods parameters.
- Some code refactoring in drivers. 
- I2C speed lowered to 100Khz instead of 1Mhz (Wire class). This to respect I2C hardware limitation related to the PullUp resistors in used with Inkplate devices.
- Added Inkplate::lightSleep() and Inkplate::deepSleep() methods, calling the InkplatePlatform class methods
- InkplatePlatform::light_sleep() and InkplatePlatform::deep_sleep() methods modified to get gpio number and level as parameters
- Inkplate::begin() method now allows for one parameter to enable/disable the SDCard initialization. Disabled by default.
- For the InkPlate-6PLUS, the Inkplate::begin() method takes a second paremeter: a handler function pointer to process touch_screen interrupts.
- Added RTC PCF85063 support.

(March 4, 2021)

Version 0.9.4: Added `PressKeys` class in support of the Buttons Extended Case as described [here](https://github.com/turgu1/InkPlate-6-Extended-Case). To be used, at compile time, EXTENDED_CASE must be #defined. The `TouchKeys` class will then **not** be included.

(March 2, 2021)

Version 0.9.3: Change required by ESP-IDF Version 4.2 for the SD Card support. Thanks to Yuki Mizuno for the supplied patch.

----

A porting effort to the ESP-IDF framework for the e-Radionica InkPlate software that can be find [here](https://github.com/e-radionicacom/Inkplate-Arduino-library).

--> Work in progress. It is usable, but testing remains to be completed. <--

Look in the `examples` folder for ready-made applications. This library is conform to the PlatformIO IDE extension. All examples can be compiled using PlatformIO.

Information about the modifications is located in file **CHANGES.md**


## ESP-IDF configuration specifics for InkPlate devices

An InkPlate application requires some functionalities to be properly set up within the ESP-IDF. The following elements have been done for this project and can be used as a reference for other projects:

- **PSRAM memory management**: The PSRAM is an extension to the ESP32 memory that offers 4MB+4MB of additional RAM. The first 4MB is readily available to integrate into the dynamic memory allocation of the ESP-IDF SDK. To do so, some parameters located in the `sdkconfig` file must be set accordingly. This must be done using the menuconfig application that is part of the ESP-IDF. The following command will launch the application (the current folder must be the main folder of EPub-InkPlate):

  ```
  $ idf.py menuconfig
  ```

  The application will show a list of configuration aspects. To configure PSRAM:

  - Select `Component Config` > `ESP32-Specific` > `Support for external, SPI-Connected RAM`
  - Select `SPI RAM config` > `Initialize SPI RAM during startup`
  - Select `Run memory test on SPI RAM Initialization`
  - Select `Enable workaround for bug in SPI RAM cache for Rev 1 ESP32s`
  - Select `SPI RAM access method` > `Make RAM allocatable using malloc() as well`

  Leave the other options as they are. 

- **ESP32 processor speed**: The processor must be run at 240MHz. The following line in `platformio.ini` request this speed:

    ```
    board_build.f_cpu = 240000000L
    ```
  You can also select the speed in the sdkconfig file:

  - Select `Component config` > `ESP32-Specific` > `CPU frequency` > `240 Mhz`

- **FAT Filesystem Support**: If the application requires the usage of the micro SD card. This card must be formatted on a computer (Linux or Windows) with a FAT 32 partition. The InkPlate6Ctrl class requires this in its current configuration. The following parameters must be adjusted in `sdkconfig`:

  - Select `Component config` > `FAT Filesystem support` > `Max Long filename length` > `255`
  - Select `Number of simultaneously open files protected  by lock function` > `5`
  - Select `Prefer external RAM when allocating FATFS buffer`

- **Flash memory partitioning (optional)**: the file `partitions.csv` contains the table of partitions definition for the 4MB flash memory, required to support the largest applications size in an OTA context. The partitions OTA_0 and OTA_1 have been set to be 1.3MB in size. In the `platformio.ini` file, the line `board_build.partitions=...` is directing the use of these partitions configuration. 
 
