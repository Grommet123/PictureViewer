/* Picture Viewer

    By GK Grotsky
    8/14/16
*/
#ifndef PictureViewer_h
#define PictureViewer_h

#include <SPI.h>
#include <SD.h>
#include <Arduino.h>

/*
Picture Viewer displays bitmap (bmp) files on a 1.8” TFT display.
The files are read from a SD card. Both the TFT display and the SD card
use the SPI protocol. The files can be displayed  randomly or sequentially.
Note: The file names must have the format picxx.bmp, where xx is a number
from 1 to 99.

   The SD circuit:
    SD card attached to SPI bus as follows:
 ** MOSI - pin 11 on Arduino Uno/Duemilanove/Diecimila
 ** MISO - pin 12 on Arduino Uno/Duemilanove/Diecimila
 ** CLK -  pin 13 on Arduino Uno/Duemilanove/Diecimila
 ** CS -   Pin 4  on Arduino Uno/Duemilanove/Diecimila

   The TFT circuit:
    TFT display attached to SPI bus as follows:
 ** MOSI - pin 11 on Arduino Uno/Duemilanove/Diecimila
 ** MISO - pin 12 on Arduino Uno/Duemilanove/Diecimila
 ** CLK -  pin 13 on Arduino Uno/Duemilanove/Diecimila
 ** RS  -  pin 8  on Arduino Uno/Duemilanove/Diecimila
 ** DC -   Pin 9  on Arduino Uno/Duemilanove/Diecimila
 ** CS -   Pin 10 on Arduino Uno/Duemilanove/Diecimila 
*/
#define VERSION  "3.2"        // Version number
#define TFT_CS  10            // Chip select line for TFT display
#define TFT_DC   9            // Data/command line for TFT
#define TFT_RST  8            // Reset line for TFT
#define SD_CS    4            // Chip select line for SD card
#define TEST_PIN 5            // Test select switch
#define MODE_PIN 6            // Mode select switch
#define DELAY_TIME 5000       // Delay time between pictures
#define TESTPICTURE "pic14.bmp" // Test picture for debugging
#define BUFFPIXEL 20

enum displayModeEnum {
  Test,
  NonTest
};

boolean bmpDraw(char *filename, uint8_t x, uint8_t y);
uint16_t read16(File f);
uint32_t read32(File f);
void displaySplashScreen(displayModeEnum displayMode = NonTest);
void displayFileName(char *fileName, boolean fileFound = true, int bmpWidth = 0, int bmpHeight = 0);

#endif  // #define PictureViewer_h
