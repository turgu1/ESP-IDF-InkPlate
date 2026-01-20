/*
   Basic_monochorme example for e-radionica.com Inkplate devices
   For this example you will need only USB cable and Inkplate.
   Select "Inkplate 6(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 6(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/

   This example will show you how you can draw some simple graphics using
   Adafruit GFX functions. Yes, Inkplate library is 100% compatible with GFX lib!
   Learn more about Adafruit GFX: https://learn.adafruit.com/adafruit-gfx-graphics-library )

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   15 July 2020 by e-radionica.com
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include <iostream>
#include <string>
#include <math.h>

#include "inkplate.hpp"
#include "logo.hpp"

Inkplate display(DisplayMode::INKPLATE_1BIT);

static const char * TAG = "Main";

uint16_t w;
uint16_t h;

// Small function that will write on the screen what function is currently in demonstration.
void displayCurrentAction(std::string text)
{
  display.setTextSize(2);
  display.setCursor(2, h - 20);
  display.print(text);
}

int random(int a, int b) 
{
  // a -> 0
  // b -> RAND_MAX

  long long r = std::rand();
  return (a + (r * b) / RAND_MAX);
}

void mainTask(void * params) 
{
  // Show a downcount of 10 seconds at the usb port
  for (int i = 10; i > 0; i--) {
    std::cout << "\r" << i << "..." << std::flush;
    ESP::delay(1000);
  }
  std::cout << std::endl << std::flush;

  ESP_LOGI(TAG, "Initialization.");

  display.begin();
  display.clearDisplay();
  display.display();

  w = display.width();
  h = display.height();

  ESP_LOGI(TAG, "Display size: width: %d, height: %d", w, h);

  display.setCursor(150, h / 2);
  display.setTextSize(4);

  #if INKPLATE_6
    display.print("Welcome to Inkplate 6!");
  #elif INKPLATE_6PLUS
    display.print("Welcome to Inkplate 6PLUS!");
  #else
    display.print("Welcome to Inkplate 10!");
  #endif

  display.display(); // Write hello message
  ESP::delay(5000);

  for (;;) { 
    display.setRotation(0);
    display.setTextColor(BLACK, WHITE);

    // Example will demostrate funcionality one by one. You always first set everything in the frame buffer and
    // afterwards you show it on the screen using display.display().

    // Let's start by drawing a pixel at x = 100 and y = 50 location
    display.clearDisplay(); // Clear everytning that is inside frame buffer in ESP32
    displayCurrentAction(
        "Drawing a pixel"); // Function which writes small text at bottom left indicating what's currently done
                            // NOTE: you do not need displayCurrentAction function to use Inkplate!
    display.drawPixel(100, 50, BLACK); // Draw one black pixel at X = 100, Y = 50 position in BLACK color (must be black
                                        // since Inkplate is in BW mode)
    display.display(); // Send image to display. You need to call this one each time you want to transfer frame buffer
                       // to the screen.
    ESP::delay(5000);

    // -----

    // Now, let's draw some random pixels!
    display.clearDisplay(); // Clear everything that is inside frame buffer in ESP32
    for (int i = 0; i < 600; i++)
    { // Write 600 black pixels at random locations
        display.drawPixel(random(0, w - 1), random(0, h - 1), BLACK);
    }
    displayCurrentAction("Drawing 600 random pixels");
    display.display(); // Write everything from frame buffer to screen
    ESP::delay(5000);

    // -----

    // Draw two diagonal lines accros screen
    display.clearDisplay();
    display.drawLine(
        0, 0, w - 1, h - 1,
        BLACK); // All of those drawing fuctions originate from Adafruit GFX library, so maybe you are already familiar
    display.drawLine(w - 1, 0, 0, h - 1, BLACK); // with those. Arguments are: start X, start Y, ending X, ending Y, color.
    displayCurrentAction("Drawing two diagonal lines");
    display.display();
    ESP::delay(5000);

    // -----

    // And again, let's draw some random lines on screen!
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
    {
        display.drawLine(random(0, w - 1), random(0, h - 1), random(0, w - 1), random(0, h - 1), BLACK);
    }
    displayCurrentAction("Drawing 50 random lines");
    display.display();
    ESP::delay(5000);

    // -----

    // Let's draw some random thick lines on screen!
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
    {
        display.drawThickLine(random(0, w - 1), random(0, h - 1), random(0, w - 1), random(0, h - 1), BLACK,
                              (float)random(1, 20));
    }
    displayCurrentAction("Drawing 50 random lines");
    display.display();
    ESP::delay(5000);

    // -----

    // Now draw one horizontal...
    display.clearDisplay();
    display.drawFastHLine(100, 100, w - 200, BLACK); // Arguments are: starting X, starting Y, length, color
    displayCurrentAction("Drawing one horizontal line");
    display.display();
    ESP::delay(5000);

    // -----

    //... and one vertical line
    display.clearDisplay();
    display.drawFastVLine(100, 100, h - 200, BLACK); // Arguments are: starting X, starting Y, length, color
    displayCurrentAction("Drawing one vertical line");
    display.display();
    ESP::delay(5000);

    // -----

    // Now, let' make a grid using only horizontal and vertical lines
    display.clearDisplay();
    for (int i = 0; i < w; i += 8)
    {
        display.drawFastVLine(i, 0, h, BLACK);
    }
    for (int i = 0; i < h; i += 4)
    {
        display.drawFastHLine(0, i, w, BLACK);
    }
    displayCurrentAction("Drawing a grid using horizontal and vertical lines");
    display.display();
    ESP::delay(5000);

    // -----

    // Draw rectangle at X = 200, Y = 200 and size of 400x300 pixels
    display.clearDisplay();
    display.drawRect(200, 200, 400, 300, BLACK); // Arguments are: start X, start Y, size X, size Y, color
    displayCurrentAction("Drawing rectangle");
    display.display();
    ESP::delay(5000);

    // -----

    // Draw rectangles on random location, size 100x150 pixels
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
    {
        display.drawRect(random(0, w - 1), random(0, h - 1), 100, 150, BLACK);
    }
    displayCurrentAction("Drawing many rectangles");
    display.display();
    ESP::delay(5000);

    // -----

    // Draw filled black rectangle at X = 200, Y = 200, size of 400x300 pixels
    display.clearDisplay();
    display.fillRect(200, 200, 400, 300, BLACK); // Arguments are: start X, start Y, size X, size Y, color
    displayCurrentAction("Drawing black rectangle");
    display.display();
    ESP::delay(5000);

    // -----

    // Draw filled black rectangles on random location, size of 30x30 pixels
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
    {
        display.fillRect(random(0, w - 1), random(0, h - 1), 30, 30, BLACK);
    }
    displayCurrentAction("Drawing many filled rectangles randomly");
    display.display();
    ESP::delay(5000);

    // -----

    // Draw circle at center of a screen with radius of 75 pixels
    display.clearDisplay();
    display.drawCircle(w / 2, h / 2, 75, BLACK); // Arguments are: start X, start Y, radius, color
    displayCurrentAction("Drawing a circle");
    display.display();
    ESP::delay(5000);

    // -----

    // Draw some circles at random location with radius of 25 pixels
    display.clearDisplay();
    for (int i = 0; i < 40; i++)
    {
        display.drawCircle(random(0, w - 1), random(0, h - 1), 25, BLACK);
    }
    displayCurrentAction("Drawing many circles randomly");
    display.display();
    ESP::delay(5000);

    // -----

    // Draw black filled circle at center of a screen with radius of 75 pixels
    display.clearDisplay();
    display.fillCircle(w / 2, h / 2, 75, BLACK); // Arguments are: start X, start Y, radius, color
    displayCurrentAction("Drawing black-filled circle");
    display.display();
    ESP::delay(5000);

    // -----

    // Draw some black filled circles at random location with radius of 15 pixels
    display.clearDisplay();
    for (int i = 0; i < 40; i++)
    {
        display.fillCircle(random(0, w - 1), random(0, h - 1), 15, BLACK);
    }
    displayCurrentAction("Drawing many filled circles randomly");
    display.display(); // To show stuff on screen, you always need to call display.display();
    ESP::delay(5000);

    // -----

    // Draw rounded rectangle at X = 200, Y = 200 and size of 400x300 pixels and radius of 10 pixels
    display.clearDisplay();
    display.drawRoundRect(200, 200, 400, 300, 10,
                          BLACK); // Arguments are: start X, start Y, size X, size Y, radius, color
    displayCurrentAction("Drawing rectangle with rounded edges");
    display.display();
    ESP::delay(5000);

    // -----

    // Draw rounded rectangles on random location, size 100x150 pixels, radius of 5 pixels
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
    {
        display.drawRoundRect(random(0, w - 1), random(0, h - 1), 100, 150, 5, BLACK);
    }
    displayCurrentAction("Drawing many rounded edges rectangles");
    display.display();
    ESP::delay(5000);

    // -----

    // Draw filled black rect at X = 200, Y = 200, size of 400x300 pixels and radius of 10 pixels
    display.clearDisplay();
    display.fillRoundRect(200, 200, 400, 300, 10,
                          BLACK); // Arguments are: start X, start Y, size X, size Y, radius, color
    displayCurrentAction("This is filled rectangle with rounded edges");
    display.display();
    ESP::delay(5000);

    // -----

    // Draw filled black rects on random location, size of 30x30 pixels, radius of 3 pixels
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
    {
        display.fillRoundRect(random(0, w - 1), random(0, h - 1), 30, 30, 3, BLACK);
    }
    displayCurrentAction("Random rounded edge filled rectangles");
    display.display();
    ESP::delay(5000);

    // -----

    // Draw simple triangle
    display.clearDisplay();
    display.drawTriangle(250, 400, 550, 400, 400, 100, BLACK); // Arguments are: X1, Y1, X2, Y2, X3, Y3, color
    display.display();
    ESP::delay(5000);

    // -----

    // Draw filled triangle inside simple triangle (so no display.clearDisplay() this time)
    display.fillTriangle(300, 350, 500, 350, 400, 150, BLACK); // Arguments are: X1, Y1, X2, Y2, X3, Y3, color
    displayCurrentAction("Drawing filled triangle inside exsisting one");
    display.display();
    ESP::delay(5000);

    // -----

    // Display some bitmap on screen. We are going to display e-radionica logo on display at location X = 200, Y = 200
    // Image is 576x100 pixels and we want to every pixel of this bitmap to be black.
    display.clearDisplay();
    display.drawImage(logo, (w / 2) - (576 / 2), (h / 2) - 50, 576, 100,
                      BLACK); // Arguments are: array variable name, start X, start Y, size X, size Y, color
    displayCurrentAction("Drawing e-radionica.com logo");
    display.display();
    ESP::delay(5000);

    // -----
    
    // Write some text on screen with different sizes
    display.clearDisplay();
    for (int i = 0; i < 6; i++)
    {
        display.setTextSize(i +
                            1); // textSize parameter starts at 0 and goes up to 10 (larger won't fit Inkplate 6 screen)
        display.setCursor(200, (i * i * 8)); // setCursor works as same as on LCD displays - sets "the cursor" at the
                                             // place you want to write someting next
        #if INKPLATE_6
          display.print("INKPLATE 6!");        // The actual text you want to show on e-paper as String
        #elif INKPLATE_6PLUS
          display.print("INKPLATE 6PLUS!");        // The actual text you want to show on e-paper as String
        #else
          display.print("INKPLATE 10!");        // The actual text you want to show on e-paper as String
        #endif
    }
    displayCurrentAction("Text in different sizes and shadings");
    display.display(); // To show stuff on screen, you always need to call display.display();
    ESP::delay(5000);

    // -----

    // Write same text on different location, but now invert colors (text is white, text background is black), without
    // cleaning the previous text
    display.setTextColor(
        WHITE, BLACK); // First argument is text color, while second argument is background color. In BW, there are
    for (int i = 0; i < 6; i++)
    { // only two options: BLACK & WHITE
        display.setTextSize(i + 1);
        display.setCursor(200, 300 + (i * i * 8));
        #if INKPLATE_6
          display.print("INKPLATE 6!");
        #elif INKPLATE_6PLUS
          display.print("INKPLATE 6PLUS!");        // The actual text you want to show on e-paper as String
        #else
          display.print("INKPLATE 10!");
        #endif
    }
    display.display();
    display.setTextColor(BLACK, WHITE);
    ESP::delay(5000);

    // -----

    // Draws an elipse with x radius, y radius, center x, center y and color
    display.clearDisplay();
    display.drawElipse(100, 200, w / 2, h / 2, BLACK);
    displayCurrentAction("Drawing an elipse");
    display.display();
    ESP::delay(5000);

    // -----

    // Fills an elipse with x radius, y radius, center x, center y and color
    display.clearDisplay();
    display.fillElipse(100, 200, w / 2, h / 2, BLACK);
    displayCurrentAction("Drawing a filled elipse");
    display.display();
    ESP::delay(5000);

    // -----

    // Code block for generating random points and sorting them in a counter
    // clockwise direction.
    int xt[10];
    int yt[10];
    int n = 10;
    for (int i = 0; i < n; ++i)
    {
        xt[i] = random(100, w - 100);
        yt[i] = random(100, h - 100);
    }
    int k;
    for (int i = 0; i < n - 1; ++i)
        for (int j = i + 1; j < n; ++j)
            if (atan2(yt[j] - (h / 2), xt[j] - (w / 2)) < atan2(yt[i] - (h / 2), xt[i] - (w / 2)))
            {
                k = xt[i], xt[i] = xt[j], xt[j] = k;
                k = yt[i], yt[i] = yt[j], yt[j] = k;
            }

    // Draws a polygon, from x and y coordinate arrays of n points in color c
    display.clearDisplay();
    display.drawPolygon(xt, yt, n, BLACK);
    displayCurrentAction("Drawing a polygon");
    display.display();
    ESP::delay(5000);

    // -----

    // Fills a polygon, from x and y coordinate arrays of n points in color c,
    // Points need to be counter clockwise sorted
    // Method can be quite slow for now, probably will improve
    display.clearDisplay();
    display.fillPolygon(xt, yt, n, BLACK);
    displayCurrentAction("Drawing a filled polygon");
    display.display();
    ESP::delay(5000);

    // -----

    // Write text and rotate it by 90 deg. Four times
    display.setTextSize(8);
    display.setTextColor(WHITE, BLACK);
    for (int r = 0; r < 4; r++)
    {
      display.setCursor(100, 100);
      display.clearDisplay();
      display.setRotation(
          r); // Set rotation will sent rotation for the entire display, so you can use it sideways or upside-down
      #if INKPLATE_6
        display.print("INKPLATE 6");
      #elif INKPLATE_6PLUS
        display.print("INKPLATE 6PLUS");        // The actual text you want to show on e-paper as String
      #else
        display.print("INKPLATE 10");
      #endif
      display.display();
      ESP::delay(5000);
    }
    display.setTextColor(BLACK, WHITE);

  } // Loop forever
}


#define STACK_SIZE 10000

extern "C" {

  void app_main()
  {
    TaskHandle_t xHandle = NULL;

    xTaskCreate(mainTask, "mainTask", STACK_SIZE, (void *) 1, tskIDLE_PRIORITY, &xHandle);
    configASSERT(xHandle);
  }

} // extern "C"