 
/*
   Inkplate_Waveform_EEPROM_Programming example for e-radionica.com Inkplate 10
   For this example you will need only USB cable and Inkplate 10.
   Select "Inkplate 10(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 10(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/

   This example will program a new waveform in ESP32 EEPROM if the waveform has been accidentally overwritten.
   Upload code and with Touchpads 1 and 3 select waveform. It will show gradient lines on the display.
   They should go from dark to light. Select the one that looks the best to you.
   Select Touchpad 2 to select waveform and burn it into ESP32 EEPROM.
   CAUTION! Changing EEPROM size can wipe waveform data.
   Waveform data is stored in EEPROM on address range form address 0 to address 75.

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   13 March 2022 by e-radionica.com
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef INKPLATE_10
  #error "Wrong board selection for this example, please select Inkplate 10 platformio.ini."
#endif

#include "inkplate.hpp" // Include Inkplate library to the sketch

Inkplate display(DisplayMode::INKPLATE_3BIT); // Create object on Inkplate library and set library to work in grayscale mode

// All waveforms for Inkplate 10 board
uint8_t waveform1[8][9] = {{0, 0, 0, 0, 0, 0, 0, 1, 0}, {0, 0, 0, 2, 2, 2, 1, 1, 0}, {0, 0, 2, 1, 1, 2, 2, 1, 0},
                           {0, 1, 2, 2, 1, 2, 2, 1, 0}, {0, 0, 2, 1, 2, 2, 2, 1, 0}, {0, 2, 2, 2, 2, 2, 2, 1, 0},
                           {0, 0, 0, 0, 0, 2, 1, 2, 0}, {0, 0, 0, 2, 2, 2, 2, 2, 0}};
uint8_t waveform2[8][9] = {{0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 2, 1, 2, 1, 1, 0}, {0, 0, 0, 2, 2, 1, 2, 1, 0},
                           {0, 0, 2, 2, 1, 2, 2, 1, 0}, {0, 0, 0, 2, 1, 1, 1, 2, 0}, {0, 0, 2, 2, 2, 1, 1, 2, 0},
                           {0, 0, 0, 0, 0, 1, 2, 2, 0}, {0, 0, 0, 0, 2, 2, 2, 2, 0}};
uint8_t waveform3[8][9] = {{0, 3, 3, 3, 3, 3, 3, 3, 0}, {0, 1, 2, 1, 1, 2, 2, 1, 0}, {0, 2, 2, 2, 1, 2, 2, 1, 0},
                           {0, 0, 2, 2, 2, 2, 2, 1, 0}, {0, 3, 3, 2, 1, 1, 1, 2, 0}, {0, 3, 3, 2, 2, 1, 1, 2, 0},
                           {0, 2, 1, 2, 1, 2, 1, 2, 0}, {0, 3, 3, 3, 2, 2, 2, 2, 0}};
uint8_t *waveformList[] = {&waveform1[0][0], &waveform2[0][0], &waveform3[0][0]};

// Calculate number of possible waveforms
uint8_t waveformListSize = (sizeof(waveformList) / sizeof(uint8_t *));

// Struct for reading waveform from EEPROM memory of ESP32
waveformData waveformEEPROM;

int currentWaveform = 0;

void showGradient(int _selected);

void mainTask(void * params)
{
    display.begin();   // Init library (you should call this function ONLY ONCE)

    while (true) {
        // Display all shades of gray on epaper with current waveform
        showGradient(currentWaveform);

        // Until "Load" key is not pressed, user can select one of the waveforms
        while (!display.readTouchpad(PAD2))
        {
            // Select and show next waveform
            if (display.readTouchpad(PAD3))
            {
                currentWaveform++;
                if (currentWaveform > waveformListSize - 1)
                    currentWaveform = 0;
                showGradient(currentWaveform);
            }

            // Select and show prev. waveform
            if (display.readTouchpad(PAD1))
            {
                currentWaveform--;
                if (currentWaveform < 0)
                    currentWaveform = waveformListSize - 1;
                showGradient(currentWaveform);
            }
        }

        // Load waveform in EEPROM memory of ESP32
        waveformEEPROM.waveform_id = INKPLATE10_WAVEFORM1 + currentWaveform;
        memcpy(&waveformEEPROM.waveform, waveformList[currentWaveform], sizeof(waveformEEPROM.waveform));
        //waveformEEPROM.checksum = display.calculateChecksum(waveformEEPROM);
        display.burnWaveformToEEPROM(&waveformEEPROM);

        // Show message on the display
        display.clearDisplay();
        display.setCursor(10, 100);
        display.print("Waveform");
        display.print(currentWaveform + 1, DEC);
        display.print(" selected & programmed into ESP32 EEPROM");
        display.display();

        ESP::delay(5000);
    }
}

// Prints gradient lines and currently selected waveform
void showGradient(int _selected)
{
    int w = display.width() / 8;
    int h = display.height() - 100;

    display.changeWaveform(waveformList[currentWaveform]);

    display.fillRect(0, 725, 1200, 100, 7);

    display.setTextSize(4);
    display.setTextColor(0);
    display.setCursor(420, 743);
    display.print("Waveform select");
    display.setCursor(432, 792);
    display.print("Prev Load Next");

    display.setCursor(800, 743);
    display.print("1 2 3");
    display.drawRect((_selected * 6 * 4 * 2) + 800 - 3, 740, (6 * 4) + 2, (8 * 4) + 2, 0);

    for (int i = 0; i < 8; i++)
    {
        display.fillRect(i * w, 0, w, h, i);
    }
    display.display();
}


#define STACK_SIZE 20000

extern "C" {

  void app_main()
  {
    TaskHandle_t xHandle = NULL;

    xTaskCreate(mainTask, "mainTask", STACK_SIZE, (void *) 1, tskIDLE_PRIORITY, &xHandle);
    configASSERT(xHandle);
  }

} // extern "C"