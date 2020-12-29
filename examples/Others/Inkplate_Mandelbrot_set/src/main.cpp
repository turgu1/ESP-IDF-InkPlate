/*
   Inkplate_Mandelbrot_set sketch for e-radionica.com Inkplate devices
   Select "Inkplate 6(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 6(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/

    This example renders the mandelbrot set to coordiantes to Inkplate.
    Due to the nature of Mandelbrot set, it is quite slow on low powered MCUs, so please be patient :)

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   15 July 2020 by e-radionica.com
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logging.hpp"

#include "inkplate.hpp"

#define MAXITERATIONS 150

static const char * TAG = "Mandelbrot";

Inkplate display(DisplayMode::INKPLATE_1BIT);

// Takes a long time to render, cca 3 minutes

// Explore different positions to draw
// Some interesting ones can be found here http://www.cuug.ab.ca/dewara/mandelbrot/Mandelbrowser.html

double xFrom = -0.7423, xTo = -0.8463;
double yFrom = 0.1092, yTo = 0.2102;

struct complex
{
  double re;
  double im;
};

void delay(int msec) { vTaskDelay(msec / portTICK_PERIOD_MS); }

void addComplex(struct complex *z, struct complex *c)
{
  z->re += c->re;
  z->im += c->im;
}

void squareComplex(struct complex *z)
{
  double re = z->re;
  double im = z->im;
  z->re = re * re - im * im;
  z->im = 2 * re * im;
}

double modulusComplexSqr(struct complex *z)
{
  return z->re * z->re + z->im * z->im;
}

uint8_t colorAt(double x, double y)
{
  struct complex z = {0.0, 0.0};
  struct complex c = {x, y};

  int i;
  for (i = 0; i < MAXITERATIONS && modulusComplexSqr(&z) <= 4.0; ++i)
  {
    squareComplex(&z);
    addComplex(&z, &c);
  }
  return i == MAXITERATIONS;
}

void mandelbrot_task(void * param)
{
  display.begin();
  display.clearDisplay();
  display.display();

  for(;;) {
    display.clearDisplay();
    for (int j = 0; j < display.height(); ++j)
    {
      for (int i = 0; i < display.width(); ++i)
        display.drawPixel(
          i, j, colorAt(xFrom + (double)i * (xTo - xFrom) / 800.0, yFrom + (double)j * (yTo - yFrom) / 600.0));
      // for whole set:
      // display.drawPixel(i, j, colorAt(-2.0 + (3.0 * (double)i / 800.0), -1.0 + 2.0 * (double)j / 600.0));
      ESP_LOGI(TAG, "%d", j);
    }
    display.display();
    delay(5000);
  }
}

#define STACK_SIZE 10000

extern "C" {

  void app_main()
  {
    TaskHandle_t xHandle = NULL;

    xTaskCreate(mandelbrot_task, "mainTask", STACK_SIZE, (void *) 1, tskIDLE_PRIORITY, &xHandle);
    configASSERT(xHandle);
  }

} // extern "C"