/*
#include <Arduino.h>
#include "config.h"

#define LEDPIN D3


#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (D5)
#define PN532_MOSI (D7)
#define PN532_SS   (D4)
#define PN532_MISO (D6)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

// Uncomment just _one_ line below depending on how your breakout or shield
// is connected to the Arduino:

// Use this line for a breakout with a software SPI connection (recommended):
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
// Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Use this line for a breakout with a hardware SPI connection.  Note that
// the PN532 SCK, MOSI, and MISO pins need to be connected to the Arduino's
// hardware SPI SCK, MOSI, and MISO pins.  On an Arduino Uno these are
// SCK = 13, MOSI = 11, MISO = 12.  The SS line can be any digital IO pin.
//Adafruit_PN532 nfc(PN532_SS);

// Or use this line for a breakout or shield with an I2C connection:
//Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define SDA D2
#define SCL D1
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

#define XPOS   0 // Indexes into the 'icons' array in function below
#define YPOS   1
#define DELTAY 2

bool compareUUID(uint8_t uid_A[CONFIG_UUID_LENGTH], uint8_t uid_B[CONFIG_UUID_LENGTH], uint8_t uidLength){
  for (int i = 0; i < uidLength; i++)
  {
    if (uid_A[i] != uid_B[i])
    {
      return false;
    }
  }
  return true;
}

int checkforUUIDinArray(uint8_t uid_A[MAX_PROJECTS][CONFIG_UUID_LENGTH], uint8_t uid_B[CONFIG_UUID_LENGTH], uint8_t uidLength){
  for (int i = 0; i < MAX_PROJECTS; i++)
  {
    uint8_t temp[CONFIG_UUID_LENGTH];
    for (int j = 0; j < CONFIG_UUID_LENGTH; j++)
    {
      temp[i] = uid_A[i][j];
      if(compareUUID(temp,uid_B,uidLength)){
        return i;
      }
    }
  }
  return -1;
}


void convertSeconds(int totalSeconds, int *minutes, int *seconds) {
    *minutes = totalSeconds / 60;       // Calculate minutes
    *seconds = totalSeconds % 60;       // Calculate remaining seconds
}

void TimerScreen(int time, int stage){
  display.clearDisplay();
  if (CONFIG_FLIP_SCREEN_FOR_TIMER)
  {
    display.setRotation(2); // flip the screen so the hackers can see it
  }
  display.setTextSize(4);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  
  int minutes = 0;
  int seconds = 0;
  char group = 'A';
  if (stage == 1)
  {
    group = 'A';
  }
  else if (stage == 2)
  {
    group = 'B';
  }

  convertSeconds(time, &minutes, &seconds);

  display.printf(
    "%d:%02d%c", 
    minutes,
    seconds,
    group
  );

  display.display();
}


void showTables(int tables[], bool tablesdone[], int numberoftables, bool timeout, int timeleft, bool started){
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner

  for (int i = 0; i < numberoftables; i++)
  {
    if (tablesdone[i] == true)
    {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    }
    display.printf(
      "%04d",
      tables[i]
    );
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // Draw 'normal' text
    display.print("   ");
    
    if (i == 2)
    {
      if (numberoftables < 7)
      {
        display.println(" ");
      }
    }
  }
  if (started)
  {
    display.setCursor(14*6, 24);
    if (timeout)
    {
      display.print("TIMEOUT");
    }
    else
    {
      int minutes = 0;
      int seconds = 0;

      convertSeconds(timeleft, &minutes, &seconds);
      display.printf(
        "%d:%02d", 
        minutes,
        seconds
      );
    }
  }
  display.display();
}

unsigned long timesetpoint = 0;
int pastMode = 0;
int currentMode = 0;
unsigned long timethathaspassed = 0;
int timeRemaining = 0;

void Alarm(){
  digitalWrite(LEDPIN, HIGH);
  delay(1000);
  digitalWrite(LEDPIN, LOW);
  delay(10);
}

void AlarmToggle(){
  digitalWrite(LEDPIN, HIGH);
  // every other second
  if ((millis()/1000) % 2 == 0)
  {
    // toggle pin
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  }
  delay(10);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LEDPIN, OUTPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500); // Pause for 500 miliseconds
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Clear the buffer
  display.clearDisplay();

  // nfc setup
  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }

  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);

  Serial.println("Waiting for an ISO14443A card");
}



void loop() {
  // NFC CHECK

  bool success = false;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };	// Buffer to store the returned UID
  uint8_t uidLength;				// Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)

  // if (nfc.inListPassiveTarget())
  // {
    
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

  Serial.print("SUCCESS =");
  Serial.println(success);

  if (success) {
    Serial.println("Found a card!");
    Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i=0; i < uidLength; i++)
    {
      Serial.print(" 0x");Serial.print(uid[i], HEX);
    }
    Serial.println("");

    // bool compare_check = compareUUID(uid,assignedtablesUUID[0],uidLength);

    // Serial.println(compare_check);
    // Wait 1 second before continuing
  }
  else
  {
    // PN532 probably timed out waiting for a card
    Serial.println("Timed out waiting for a card");
  }
  // }
  delay(1000);

  // int compare_check = checkforUUIDinArray(assignedtablesUUID,uid,uidLength);

  Serial.println("test");

  // if (compare_check != -1)
  // {
  //   currentMode = 1;
  //   Serial.println("UUID HIT");
  //   assignedtablescheck[compare_check] = true;
  // }

  // We just changed modes in the state machine
  // if (pastMode != currentMode)
  // {
  //   timesetpoint = millis();
  //   pastMode = currentMode;
  // }
  // timethathaspassed = millis() - timesetpoint;
  // switch (currentMode)
  // {
  //   case 0:
  //     showTables(assignedtables, assignedtablescheck, numberoftables, false, 0, false);

  //     // TODO NFC and set scanned to true
  //     break;

  //   case 1:
  //     timeRemaining = (CONFIG_PITCH_TIME_MAX - (timethathaspassed/1000)); 
  //     TimerScreen(timeRemaining, 1);
      
  //     if (timeRemaining <= 0)
  //     {
  //       currentMode = 2;
  //       Alarm();
  //     }
  //     break;

  //   case 2:
  //     timeRemaining = (CONFIG_Q_AND_A_TIME_MAX - (timethathaspassed/1000)); 
  //     TimerScreen(timeRemaining, 2);

  //     if (timeRemaining <= 0)
  //     {
  //       currentMode = 3;
  //       Alarm();
  //     }
  //     break;

  //   case 3:
  //     timeRemaining = (CONFIG_TIME_TO_NEXT_TABLE_MAX - (timethathaspassed/1000)); 
  //     showTables(assignedtables, assignedtablescheck, numberoftables, false, timeRemaining, true);

  //     if (timeRemaining <= 0)
  //     {
  //       currentMode = 4;
  //     }

  //     // TODO NFC and set scanned to true
  //     break;

  //   case 4:
  //     showTables(assignedtables, assignedtablescheck, numberoftables, true, 0, true);
  //     AlarmToggle();
  //     // TODO NFC and set scanned to true
  //     break;  
  //   default:
  //     break;
  // }
}

*/

#include <Arduino.h>
#include "config.h"

#define LEDPIN D3

/**************************************************************************/
/*!
    @file     readntag203.pde
    @author   KTOWN (Adafruit Industries)
    @license  BSD (see license.txt)

    This example will wait for any NTAG203 or NTAG213 card or tag,
    and will attempt to read from it.

    This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
    This library works with the Adafruit NFC breakout
      ----> https://www.adafruit.com/products/364

    Check out the links above for our tutorials and wiring diagrams
    These chips use SPI or I2C to communicate.

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!
*/
/**************************************************************************/
// #define PN532DEBUG
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (D5)
#define PN532_MOSI (D7)
#define PN532_SS   (D4)
#define PN532_MISO (D6)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

// Uncomment just _one_ line below depending on how your breakout or shield
// is connected to the Arduino:

// Use this line for a breakout with a software SPI connection (recommended):
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
// Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Use this line for a breakout with a hardware SPI connection.  Note that
// the PN532 SCK, MOSI, and MISO pins need to be connected to the Arduino's
// hardware SPI SCK, MOSI, and MISO pins.  On an Arduino Uno these are
// SCK = 13, MOSI = 11, MISO = 12.  The SS line can be any digital IO pin.
//Adafruit_PN532 nfc(PN532_SS);

// Or use this line for a breakout or shield with an I2C connection:
//Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define SDA D2
#define SCL D1
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

#define XPOS   0 // Indexes into the 'icons' array in function below
#define YPOS   1
#define DELTAY 2

void displayUUID(uint8_t uid[], int uuidlength){
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner

  for (int i = 0; i < uuidlength; i++)
  {
    display.printf("%d ",uid[i]);
    if (i % 5 == 0)
    {
      display.println(" ");
    }
    
  }

  display.display();
}

void displayNoUUID(){
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner

  display.println("NO CARD IN RANGE");

  display.display();
}


void convertSeconds(int totalSeconds, int *minutes, int *seconds) {
    *minutes = totalSeconds / 60;       // Calculate minutes
    *seconds = totalSeconds % 60;       // Calculate remaining seconds
}

void TimerScreen(int time, int stage){
  display.clearDisplay();
  if (CONFIG_FLIP_SCREEN_FOR_TIMER)
  {
    display.setRotation(2); // flip the screen so the hackers can see it
  }
  display.setTextSize(4);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  
  int minutes = 0;
  int seconds = 0;
  char group = 'A';
  if (stage == 1)
  {
    group = 'A';
  }
  else if (stage == 2)
  {
    group = 'B';
  }

  convertSeconds(time, &minutes, &seconds);

  display.printf(
    "%d:%02d%c", 
    minutes,
    seconds,
    group
  );
  // Serial.printf(
  //   "%d:%02d%c", 
  //   minutes,
  //   seconds,
  //   group
  // );

  display.display();
}


void showTablesa(bool hasrunoutoftime, int timeleft, bool startedclock){
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner

  for (int i = 0; i < numberoftables; i++)
  {
    if (assignedtablescheck[i] == true)
    {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    }
    display.printf(
      "%04d",
      assignedtables[i]
    );
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // Draw 'normal' text
    display.print("   ");
    
    if (i == 2)
    {
      if (numberoftables < 7)
      {
        display.println(" ");
      }
    }
  }
  if (startedclock)
  {
    display.setCursor(14*6, 24);
    if (hasrunoutoftime)
    {
      display.print("TIMEOUT");
    }
    else
    {
      int minutes = 0;
      int seconds = 0;

      convertSeconds(timeleft, &minutes, &seconds);
      display.printf(
        "%d:%02d", 
        minutes,
        seconds
      );
    }
  }
  display.display();
  // Serial.println("TEST");
}

unsigned long starttime=0;
unsigned long endtime=0;
int timeleft = 100;
int currentStage = 1;

unsigned long timesetpoint = 0;
int pastMode = 0;
int currentMode = 0;
unsigned long timethathaspassed = 0;
int timeRemaining = 0;


void Alarm(){
  digitalWrite(LEDPIN, HIGH);
  delay(1000);
  digitalWrite(LEDPIN, LOW);
  delay(10);
}

void AlarmToggle(){
  // every other second
  if ((millis()/1000) % 2 == 0)
  {
    // toggle pin
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  }
  // delay(10);
}



void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10); // for Leonardo/Micro/Zero
  Serial.println("Hello!");

  pinMode(LEDPIN, OUTPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500); // Pause for 500 miliseconds
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Clear the buffer
  display.clearDisplay();

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }

  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);

  nfc.setPassiveActivationRetries(0x01);

  Serial.println("Waiting for an ISO14443A card");
}

void loop(void) {
  boolean success = 0;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };	// Buffer to store the returned UID
  uint8_t uidLength;				// Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)

  // Serial.println("SCANNING");

  if(nfc.inListPassiveTarget())
  {
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

    if (success) {
      Alarm();
      Serial.println("Found a card!");
      Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
      Serial.print("UID Value: ");
      for (uint8_t i=0; i < uidLength; i++)
      {
        Serial.print(" 0x");Serial.print(uid[i], HEX);
      }
      Serial.println("");
      // Wait 1 second before continuing
      delay(1000);
      // displayUUID(uid,uidLength);
    }
    else
    {
      // PN532 probably timed out waiting for a card
      Serial.println("Timed out waiting for a card");
      // displayNoUUID();
    }
  }

  if (success)
  {
    currentMode = 1;
  }
  

  // We just changed modes in the state machine
  if (pastMode != currentMode)
  {
    timesetpoint = millis();
    pastMode = currentMode;
  }
  timethathaspassed = millis() - timesetpoint;
  switch (currentMode)
  {
    case 0:
      showTablesa(false, 0, false);

      // TODO NFC and set scanned to true
      break;

    case 1:
      timeRemaining = (CONFIG_PITCH_TIME_MAX - (timethathaspassed/1000)); 
      TimerScreen(timeRemaining, 1);
      // Serial.println("MODE=1");
      
      if (timeRemaining <= 0)
      {
        currentMode = 2;
        Alarm();
      }
      break;

    case 2:
      timeRemaining = (CONFIG_Q_AND_A_TIME_MAX - (timethathaspassed/1000)); 
      TimerScreen(timeRemaining, 2);

      if (timeRemaining <= 0)
      {
        currentMode = 3;
        Alarm();
      }
      break;

    case 3:
      timeRemaining = (CONFIG_TIME_TO_NEXT_TABLE_MAX - (timethathaspassed/1000)); 
      showTablesa(false, timeRemaining, true);

      if (timeRemaining <= 0)
      {
        currentMode = 4;
      }

      // TODO NFC and set scanned to true
      break;

    case 4:
      showTablesa(true, 0, true);
      AlarmToggle();
      // TODO NFC and set scanned to true
      break;  
    default:
      break;
  }
}

