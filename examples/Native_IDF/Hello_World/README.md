# Hello World in native esp-idf

The purpose of this example is to show how to use the ESP-IDF-InkPlate library without using Platformio SDK.

The program will only display a message "Hello World" in the center of the screen and then exit the program. It includes freertos, idf and inkplate headers and can stand as a simple test whether your esp-idf installation is working correctly.

**The example was tested only with ESP-IDF version 4.4.1.** However, as the library and example itself were made as universal as they could, past and future versions should work as well. If not, please open a new [issue](https://github.com/turgu1/ESP-IDF-InkPlate/issues) and let us know.

## Configuration

`sdkconfig` should be already preconfigured for the Inkplate 10 device. To change your Inkplate variant, go to `Component config --> Inkplate --> Choose your Inkplate variant` and change it accordingly.

## Installation

Either download the Inkplate library or link it as a git submodule and place it in the components/ folder.

```sh
# to make it a submodule of your repository
git submodule add -b v0.9.6 https://github.com/turgu1/ESP-IDF-InkPlate.git components/inkplate

# or

# to use a different version of the library, modify the submodule's branch accordingly
git submodule add -b master https://github.com/turgu1/ESP-IDF-InkPlate.git components/inkplate
```

## Compile and run

With `idf` loaded in your environment, you can manage your project like any other `esp-idf` project. For further details please follow the official [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-py.html).

```sh
# change your device port based on your actual device
idf.py -p COM4 build flash monitor
```