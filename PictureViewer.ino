/* Picture Viewer

    By GK Grotsky
    8/14/16
*/
#include "PictureViewer.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

int numberOfFiles = 0;
int pastMode = HIGH;
int pastTest = HIGH;
boolean SD_OK = false;

// The setup (runs once at start up)
void setup(void) {
  File root, entry;

  pinMode(TEST_PIN, INPUT);
  pinMode(MODE_PIN, INPUT);
  Serial.begin(9600);
  tft.initR(INITR_BLACKTAB);

  // Print program name nad version
  Serial.println("Picture Viewer");
  Serial.print("Version ");
  Serial.println(VERSION);

  // Initialize the SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
    SD_OK = false;
    displaySplashScreen();
    return;
  }
  Serial.println("OK!");
  SD_OK = true;

  // Count the number of files on the SD card
  root = SD.open("/");
  while (true) {
    entry = root.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    // Do not count any non-picxx.bmp files
    if ((strncmp(entry.name(), "PIC", 3) == 0)) numberOfFiles++;
    entry.close();
  }
  root.close();

  // Print # of files and mode
  Serial.print("Number of files = ");
  if ((digitalRead(TEST_PIN) == HIGH)) {
    Serial.println('1');
  }
  else {
    Serial.println(numberOfFiles);
  }

  tft.setRotation(0); // Portrait

  // Seed random number generator
  randomSeed(analogRead(A0));

  // Display splash screen
  if (digitalRead(TEST_PIN) == LOW) {
    Serial.print("Mode = ");
    Serial.println((digitalRead(MODE_PIN)) ? "Random" : "Sequential");
    Serial.println();
    pastMode = !(digitalRead(MODE_PIN));
  }
  else {
    Serial.print("Mode = ");
    Serial.println("TEST");
    Serial.println();
    pastTest = !(digitalRead(TEST_PIN));
  }
  pastMode = digitalRead(MODE_PIN);
}  // setup

// The loop (runs forever and a day :-))
void loop() {

  char fileName[] = "pic";
  char randomNumberChar[] = "  ";
  char listNumberChar[] = "  ";
  static long lastRandomNumber = 0;
  static int fileNumber = 1;

  if (digitalRead(TEST_PIN) == LOW) {  // Used for debugging
    // Check if the mode switch has changed
    if ((digitalRead(MODE_PIN) != pastMode) || (digitalRead(TEST_PIN) != pastTest)) {
      Serial.print("Mode = ");
      Serial.println((digitalRead(MODE_PIN)) ? "Random" : "Sequential");
      Serial.println();
      // Display splash screen
      displaySplashScreen();
      pastMode = digitalRead(MODE_PIN);
      pastTest = digitalRead(TEST_PIN);
    }
    // Check fo mode
    if (digitalRead(MODE_PIN) == HIGH) {
      // Random was selected, get random number
      long randNumber = random(1, numberOfFiles + 1);
      // If random number repeats, try again
      while (randNumber == lastRandomNumber) {
        randNumber = random(1, numberOfFiles + 1);
      }
      lastRandomNumber = randNumber;
      // Convert it to a string
      sprintf(randomNumberChar, "%d", randNumber);
      strncat(fileName, randomNumberChar, 3);
      strncat(fileName, ".bmp", 4);
      if (bmpDraw(fileName, 0, 0)) {
        delay(DELAY_TIME);
        fileNumber = 1;
      }
    }
    // Sequential was selected, dislpay files from begining to end
    else {
      // Convert list of files to a string
      sprintf(listNumberChar, "%d", fileNumber);
      strncat(fileName, listNumberChar, 3);
      strncat(fileName, ".bmp", 4);
      if (bmpDraw(fileName, 0, 0)) delay(DELAY_TIME);
      fileNumber++;
      if (fileNumber > numberOfFiles) fileNumber = 1;
    }
  }
  else { // Uesd to debug a bmp file
    if (digitalRead(TEST_PIN) != pastTest) {
      displaySplashScreen(Test);
      fileNumber = 1;
      pastTest = digitalRead(TEST_PIN);
    }
    if (bmpDraw(TESTPICTURE, 0, 0)) delay(DELAY_TIME);
  }
}  // loop

boolean bmpDraw(char *filename, uint8_t x, uint8_t y) {
  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3 * BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0;

  if ((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print("File not found");
    displayFileName(filename, false);
    return (false);
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print("File size: ");
    Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print("Image Offset: ");
    Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: ");
    Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print("Bit Depth: ");
      Serial.println(bmpDepth);
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print("Image size: ");
        Serial.print(bmpWidth);
        Serial.print("W ");
        Serial.print("x ");
        Serial.print(bmpHeight);
        Serial.print('H');
        Serial.println(" Pixels");

        displayFileName(filename, true, bmpWidth, bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if (bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if ((x + w - 1) >= tft.width())  w = tft.width()  - x;
        if ((y + h - 1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);

        for (row = 0; row < h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if (bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col = 0; col < w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r, g, b));
          } // end pixel
        } // end scanline
      } // end goodBmp
    }
  }

  bmpFile.close();
  if (!goodBmp) {
    Serial.println("BMP format not recognized.");
    return (false);
  }
  return (true);
}  // bmpDraw

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

// Displays the splach screen
void displaySplashScreen(displayModeEnum displayMode) {
  tft.setTextWrap(false);
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 5);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(2);
  tft.println("Picture");
  tft.setCursor(15, 20);
  tft.println("Viewer");
  tft.setTextSize(1);
  tft.setCursor(20, 40);
  tft.print("Version ");
  tft.println(VERSION);
  tft.setTextSize(2);
  tft.println("");
  tft.setTextColor(ST7735_BLUE);
  tft.println("GK Grotsky");
  tft.println("");
  tft.setTextColor(ST7735_GREEN);
  tft.println("Mode:");
  if (displayMode == NonTest) {
    tft.println((digitalRead(MODE_PIN)) ? "Random" : "Sequential");
  }
  else {
    tft.println("Test");
  }
  tft.setTextColor(ST7735_CYAN);
  tft.setTextSize(1);
  if (SD_OK) {
    tft.setCursor(30, 140);
    tft.print("# of files ");
    tft.println(numberOfFiles);
    tft.setCursor(30, 150);
    tft.println("SD Card OK");
  }
  else {
    tft.setCursor(30, 150);
    tft.println("SD Card Fail");
  }
  delay(10000);
}

// Displays the file name
void displayFileName(char *fileName, boolean fileFound, int bmpWidth, int bmpHeight) {
  static long lastRandomNumber;
  long randomColor;
  unsigned int textColor[] = {
    ST7735_BLUE,
    ST7735_RED,
    ST7735_GREEN,
    ST7735_CYAN,
    ST7735_MAGENTA,
    ST7735_YELLOW,
    ST7735_WHITE
  };

  char colorType [][8] = {
    "Blue",
    "Red",
    "Green",
    "Cyan",
    "Magenta",
    "Yellow",
    "White"
  };

  randomColor =  random(0, 7);
  // If random number repeats, try again
  while (randomColor == lastRandomNumber) {
    randomColor = random(0, 7);
  }
  lastRandomNumber = randomColor;
  tft.setTextWrap(false);
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(textColor[randomColor]);
  tft.setTextSize(1);
  if (!digitalRead(TEST_PIN)) {
    if (digitalRead(MODE_PIN)) { // Not test mode
      tft.setCursor(40, 0);
      tft.println("Random");
    }
    else {
      tft.setCursor(30, 0);
      tft.println("Sequential");
    }
  } // Test mode
  else {
    tft.setCursor(50, 0);
    tft.println("Test");
  }
  tft.setCursor(0, 50);
  tft.setTextSize(2);
  tft.println("File Name:");
  tft.setCursor(10, 80);
  tft.println(fileName);
  tft.setTextSize(1);
  if (!fileFound) {
    tft.setCursor(15, 110);
    tft.println("File not found");
  }
  else {
    tft.setCursor(10, 110);
    tft.println("File Size: ");
    tft.setCursor(10, 120);
    tft.print(bmpWidth);
    tft.print("W ");
    tft.print("x ");
    tft.print(bmpHeight);
    tft.print('H');
    tft.println(" Pixels");
  }
  tft.setCursor(50, 150);
  tft.println(colorType[randomColor]);
  delay(5000);
  tft.fillScreen(ST7735_BLACK);
}
