/**
 * clear_storage.ino
 *
 * Quick sketch for zeroing out contents of FRAM or EEEPROM storage
 *
 */

#include <EEPROM.h>
#include <Adafruit_FRAM_I2C.h>
#include <string>

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define WHITE SSD1306_WHITE
#define BLACK SSD1306_BLACK
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32
#define OLED_RST -1
#define CLKDURING 1000000
#define CLKAFTER 400000

Adafruit_FRAM_I2C fram = Adafruit_FRAM_I2C();

Adafruit_SSD1306 display = Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, OLED_RST, CLKDURING, CLKAFTER);

const int framCapacity = 32000;
const int eepromCapacity = 2048;	// 2048; 3.2
// const int eepromCapacity = 1080; 	// 1080; 4.0

bool usingFram = false;

void setup() {
	setupDisplay();
	display.setCursor(8, 8);

	if (fram.begin()) {
		clearFRAM();
	} else {
		clearEEPROM();
	}

	display.clearDisplay();
	display.setCursor(20, 8);
	display.print("Storage cleared");
	display.display();
}

void loop() {
	// ...
}

void clearFRAM() {
	usingFram = true;

	for (int address = 0; address < framCapacity; address++) {
		fram.write(address, 0x0);

		printProgressUpdate(address, framCapacity);
	}
}

void clearEEPROM() {
	for (int address = 0; address < eepromCapacity; address++) {
		EEPROM.update(address, 0x0);

		printProgressUpdate(address, eepromCapacity);
	}
}

void printProgressUpdate(int current, int total) {
	// only update every KB
	if (current % 1000 != 0) {
		return;
	}

	display.clearDisplay();
	display.setCursor(4, 4);
	display.printf("Clearing %s", usingFram ? "FRAM" : "EEPROM");
	display.setCursor(4, 16);
	display.printf("Progress: %dK/%dK", current / 1000, total / 1000);
	display.display();
}

void setupDisplay() {
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  display.cp437(true);
	display.setTextColor(WHITE);
	display.setRotation(2);
	display.clearDisplay();
	display.display();
}
