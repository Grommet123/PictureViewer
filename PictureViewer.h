/* Picture Viewer

    By GK Grotsky
    8/14/16
*/
#ifndef PictureViewer_h
#define PictureViewer_h

#include <SPI.h>     // SPI library
#include <SD.h>      // SD library
#include <Arduino.h> // The basic stuff

/*
Picture Viewer displays bitmap (bmp) files on a 1.8” TFT display.
The files are read from a SD card. Both the TFT display and the SD card
use the SPI protocol. The files can be displayed  randomly or sequentially.
Note: The file names must have the format picxx.bmp, where xx is a number
from 1 to 99.

   The SD circuit (uses SPI protocol):
    SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK -  pin 13
 ** CS -   Pin 4

   The TFT circuit (uses SPI protocol):
    TFT display attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK -  pin 13 
 ** RS  -  pin 8
 ** DC -   Pin 9
 ** CS -   Pin 10
*/
#define VERSION  "5.0"        // Version number
#define TFT_CS  10            // Chip select line for TFT display
#define TFT_DC   9            // Data/command line for TFT
#define TFT_RST  8            // Reset line for TFT
#define SD_CS    4            // Chip select line for SD card
#define TEST_PIN 5            // Test select switch
#define MODE_PIN 6            // Mode select switch
#define DELAY_TIME 5 * 1000   // Delay time between pictures (seconds)
#define TESTPICTURE "pic14.bmp" // Test picture for debugging
#define BUFFPIXEL 20
enum displayModeEnum {
  Test,
  NonTest
};

bool bmpDraw(char *filename, uint8_t x, uint8_t y);
uint16_t read16(File f);
uint32_t read32(File f);
void displaySplashScreen(displayModeEnum displayMode = NonTest);
void displayFileName(char *fileName, bool fileFound = true, int bmpWidth = 0, int bmpHeight = 0);

#endif  // #define PictureViewer_h
