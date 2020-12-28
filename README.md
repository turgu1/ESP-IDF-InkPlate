# ESP-IDF-InkPlate Version 0.9.0

A porting effort to the ESP-IDF framework for the e-Radionica InkPlate software that can be find [here](https://github.com/e-radionicacom/Inkplate-Arduino-library).

--> Work in progress. Not ready yet. <--

Beware that these classes are **not** re-entrant. That means that it is not possible to use them in a multi-thread context without proper mutual exclusion access control. 

This project is a working example. You can build it using PlatformIO. 

## Modifications done to the Arduino Inkplate Source code

As you can expect, the ESP-IDF framework is quit different than the Arduino framework. The porting effort of the Inkplate source code done here is not just a transformation to make it runs, but also getting it inline with the available packages available with the ESP-IDF. Many aspects must be transformed in this regard. 

The code is also being transformed to be closer to C++ functionalities, targetting a stronger usage of the language elements that will sustain the author's software developement. A bit less C and more C++...

The source code is divided in two major groups: the low-level device drivers group, under the `inkplate_platform` class, and the *mid-level* graphical group, under the `inkplate` class. The coupling between these groups is minimal: one can take the `inkplate-platform` and build on top of it without the graphical portion. The graphical group rely on the drivers. The coupling is light as it is done through aggregation instead of inheritence.

Here is a list of the changes being done:

### Global changes

Some of these changes have been done partially and will be completed in subsequent versions.

- Give the overall drivers source code a transformation to use what is called *modern C++*, alligned with C++11 and beyond
- All .h files are renamed .hpp
- macro definitions (`#define`) are reduced to a bare minimum
- `enum class` are used everywhere possible in the drivers group
- source code filenames are all in lowercase
- all driver and support files using the following naming conventions:
  - class names are in ChamelCase
  - constants are in UPPER_CASE
  - variables and method names are in lower_case
- all graphical libraries and class remains with their Arduino naming convention
- classes inheritence are reduced to insure independance of usage of the drivers group. By design, the grapphical portion remains as defined for the Arduino framework with some internal transformation to be more inline with C++.
- A TAG is added in the protected/private portion of driver classes in support of the ESP-IDF Logging mechanism (ESP_LOGx defines)
- min() definition replaced with std::min()
- _swap...() definitions replaced with std::swap()
- String replaced with std::string (No PROGMEM support required with the ESP32)

### defines.hpp

The content is now at a bare minimum: DisplayMode enum class that defines both INKPLATE_1BIT and INKPLATE_3BIT modes. Also, BLACK and WHITE values for INKPLATE_1BIT mode.

As the DisplayMode is an enum class, it is then required to access the values as follow:

- DisplayMode::INKPLATE_1BIT
- DisplayMode::INKPLATE_3BIT
  
### graphics (.hpp, .cpp)

The class is now the owner of the _partial and DMemory4Bit frame buffers. As such, methods that where defined in Inkplate.hpp are relocated into that class, namely:

- clearDisplay()
- display()
- preloadScreen()
- partialUpdate()

They select the appropriate frame buffer and call the e_ink class methods.

This change is mostly transparent for the user application.
  
### SdCard

- The module is used only to initialize the ESP-IDF drivers (SPI and FAT filesystem are used). The card can then be accessed through the standard C++/C capabilities. All filenames located on the card must be prefixed with `/sdcard/`.
  
### Image (image.hpp, image.cpp)

- the `drawXXXXFromWeb(WiFiClient *s, ...)` methods are no longer available. WiFiCLient doesn't exists in an ESP-IDF context.
- the `bool drawXXXXFromSd(SdFile *p, ...)` methods are renamed `bool drawXXXXFromFile(FILE *p, ...)`. This allow for accessing files located in a SPIFFS partition (using `/spiffs/` filename prefix) as well as SDCard files (using `/sdcard/` filename prefix) or any other file system integrated with ESP-IDF.
- functions-like related to image / pixel manipulation present in `defines.h` have been migrated here. Namely: RED, BLUE, GREEN, READ32, READ16, ROWSIZE, RGB8BIT, RGB3BIT. They are now inline functions with appropriate types named red, blue, green, read32, read16, rowSize, rgb8Bit, rgb3Bit.
  
### mcp23017 (.hpp, .cpp)

This class is implementing a generic MCP23017 driver. It is instanciated in the inkplate_platform.hpp, depending on the type of Inkplate device:

- as mcp_int (for all Inkplate devices) 
- as mcp_ext (for the Inkplate-10, and Inkplate-6).

### battery, touch_keys (.hpp, .cpp)

Those classes implement basic access to the battery and touch keys state.

### system class renamed InkplatePlatform

This name reflect more what it is. This class will is currently under eavy changes.

### FrameBuffer classes

A hierarchy of frame buffer classes has been added. These allow for flexible adaptation to the different
geometry of devices and pixel sizes.

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
 