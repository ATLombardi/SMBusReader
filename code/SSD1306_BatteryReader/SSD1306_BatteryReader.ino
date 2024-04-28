/*  
SMBus Battery Management System (BMS) Reader, adapted for a SSD1306 tiny OLED display
Tested with Ironworks 1240ST which uses the TI BQ2085 SBS 1.1 compliant gas gauge
  (https://www.ti.com/product/BQ2085)

By Anthony Lombardi KN6ZZY
  https://github.com/ATLombardi/SMBusReader/

Thanks to Paul Martis W9PEM for original code and concept

28 APR 2024
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "SMBusBattery.h"

#define SERIAL_BAUD 9600

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDR 0x3C

#define OLED_RESET -1  // Reset pin # if available

#define XOR SSD1306_INVERSE
Adafruit_SSD1306 oled (SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static const unsigned char PROGMEM battery_icon[] = {
  0b00000011, 0b11000000,
  0b00000011, 0b11000000,
  0b00011111, 0b11111000,
  0b00011111, 0b11111000,
  0b00011000, 0b00011000,
  0b00011000, 0b00011000,
  0b00011000, 0b01011000,
  0b00011000, 0b10011000,
  0b00011001, 0b00011000,
  0b00011011, 0b11011000,
  0b00011000, 0b10011000,
  0b00011001, 0b00011000,
  0b00011010, 0b00011000,
  0b00011000, 0b00011000,
  0b00011111, 0b11111000,
  0b00011111, 0b11111000
};

static const unsigned char PROGMEM no_battery_icon[] = {
  0b00000000, 0b00000000,
  0b00000011, 0b11000000,
  0b00000111, 0b11100000,
  0b00001110, 0b01110000,
  0b00001100, 0b00110000,
  0b00000000, 0b00110000,
  0b00000000, 0b01110000,
  0b00000000, 0b11100000,
  0b00000001, 0b11000000,
  0b00000001, 0b10000000,
  0b00000001, 0b10000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000001, 0b10000000,
  0b00000001, 0b10000000,
  0b00000000, 0b00000000
};

enum DisplayMode {
  V_AND_A,
  GAUGE
};
DisplayMode mode = GAUGE;

void nextMode () {
  switch (mode) {
    case V_AND_A: mode = GAUGE; break;
    case GAUGE:   mode = V_AND_A; break;
  }
}

void oledBox (uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, bool filled) {
  if (filled) {
    oled.fillRect(x, y, w, h, color);
  }
  else oled.drawRect(x, y, w, h, color);
}

void oledBitmap (uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *image) {
  oled.drawBitmap(x, y, image, w, h, WHITE);
}

void setup() {
  bool foundBattery = true;

  // set up UART serial comms
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("Battery SMBus reader by Paul Martis W9PEM and Anthony Lombardi KN6ZZY"));

  // set up I2C / SMBus
  Wire.begin();
  #ifdef WIRE_HAS_TIMEOUT
    Wire.setWireTimeout(); // the default should be fine
    sbsGetData();
    if (Wire.getWireTimeoutFlag()) {
      Serial.println(F("No response from battery"));
      foundBattery = false;
    }
  #else
    Serial.println(F("I2C Timeout not available on this platform."));
  #endif

  // generate display voltage from internal 3.3V
  if (!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.print(F("W9PEM\nKN6ZZY"));

  oledBitmap(96, 8, 16, 16, battery_icon);
  if (!foundBattery) {
    oledBitmap(114, 8, 16, 16, no_battery_icon);
  }
  oled.display();
}

void loop() {
  bool batteryCommOkay = true;
  oled.clearDisplay();
  oled.setCursor(0,0);
  oled.setTextSize(2);
  oled.setTextColor(WHITE);

  #ifdef WIRE_HAS_TIMEOUT
    Wire.setWireTimeout(); // the default should be fine
    sbsGetData();
    if (Wire.getWireTimeoutFlag()) {
      Serial.println("No response from battery");
      batteryCommOkay = false;
    }
  #else
    // likely indicates a read error, but this platform doesn't support timeout
    design_capacity = sbsWord(DESIGN_CAPACITY)*CURRENT_FACTOR;
    if (design_capacity == 65534) {
      batteryCommOkay = false;
    }
  #endif

  // this likely indicates a read error
  if (!batteryCommOkay) {
    oledBitmap(96, 8, 16, 16, battery_icon);
    oledBitmap(114, 8, 16, 16, no_battery_icon);
    
    oled.display();
    return; // don't do the rest of the loop()
  }
  sbsGetData(); // refresh values
  sbsPrintData();

  switch (mode) {
    case V_AND_A:
      drawVoltageAndCurrent();
      break;
    case GAUGE:
      drawBatteryGauge();
      break;
  }
  oled.display();
  nextMode();
  delay(2000);
}

void drawVoltageAndCurrent () {
  // draw voltage
  oled.print((float)voltage/1000);
  oled.println("V");

  // draw current
  if (current > 50000) // draining
  {
    oled.print((int)(current*CURRENT_FACTOR));
    oled.println("mA");
  }
  else if (current != 0) // charging
  {
    oled.print("+");
    oled.print((int)(current*CURRENT_FACTOR));
    oled.println("mA");
  }
  else {
    oled.setTextSize(1);
    oled.print("Disconnected");
  }
}

void drawBatteryGauge () {
  int fill = (int)(relative_soc * 1.28);
  oledBox(0, 0, 127, 32, WHITE, false);
  oledBox(1, 1, fill, 30, WHITE, true);
  oled.setCursor(8, 8);
  oled.setTextColor(XOR);
  oled.setTextSize(2);
  oled.print(relative_soc);
  oled.print("%");
  oled.setCursor(72, 8);
  oled.print((int)(remaining_capacity*CURRENT_FACTOR/1000));
  oled.print("Ah");
}


