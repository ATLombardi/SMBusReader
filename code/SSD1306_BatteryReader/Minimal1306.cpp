#include "Minimal1306.h"
#include <Arduino.h>
#include <Wire.h>

#define OLED_ADDRESS 0x3C

uint8_t fontScale = 1;
uint8_t fontColor = BLACK;

void sendCommand (uint8_t command) {
  Wire.beginTransmission(OLED_ADDRESS);
  Wire.write(0x40); // COMMAND control
  Wire.write(command);
  Wire.endTransmission();
}

// 0 ~ 127
void setColumnAddress (uint8_t start, uint8_t end) {
  sendCommand(0x21); // COMMAND_COLUMN_ADDRESS
  sendCommand(start);
  sendCommand(end);
}

// 0 ~ 7
void setPageAddress(uint8_t start, uint8_t end) {
  sendCommand(0x22); // COMMAND_PAGE_ADDRESS
  sendCommand(start & 0x07);
  sendCommand(end & 0x07);
}

void sendBuffer (const uint8_t *buffer, uint8_t len) {
  Wire.beginTransmission(OLED_ADDRESS);
  Wire.write(0x00); // DATA control
  for (uint8_t byte = 0; byte < len; byte++) {
    Wire.write(buffer[byte]);
  }
  Wire.endTransmission();
}

void initDisplay () {
  sendCommand(0xAE); // Set display OFF
  sendCommand(0xD5); // set clock divide ratio / osc freq
  sendCommand(0x80); // Display clock divide ratio / osc freq
  sendCommand(0xA8); // set multiplex ratio
  sendCommand(0x1F); //   multiplex ratio for 128x32
  sendCommand(0xD3); // set display offset
  sendCommand(0x00); //   offset
  sendCommand(0x40); // set display start line
  sendCommand(0x8D); // set charge pump
  sendCommand(0x14); // charge pump (0x10 external, 0x14 internal)
  sendCommand(0x20); // set addressing mode
  sendCommand(0x00); //   horizontal
  sendCommand(0xA1); // set segment re-map
  sendCommand(0xC8); // set COM output scan direction
  sendCommand(0xDA); // set COM hardware configuration
  sendCommand(0x02); //   config
  sendCommand(0x81); // set contrast
  sendCommand(0xCF); //   contrast
  sendCommand(0xD9); // set pre-charge period
  sendCommand(0xF1); //   0x22 external, 0xF1 internal
  sendCommand(0xDB); // set VCOMH Deselect Level
  sendCommand(0x40); //   level
  sendCommand(0xA4); // set all pixels OFF
  sendCommand(0xA6); // set display not inverted
  sendCommand(0xAF); // set display ON

  setColumnAddress(0, 127);
  setPageAddress(0, 7);
}

void clearDisplay () {
  sendCommand(0xA4);
}

void setFontSize (uint8_t size) {
  fontScale = size;
}

void setFontColor (uint8_t color) {
  fontColor = color;
}

void drawRect (uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, bool filled) {
  
}

void drawBitmap (uint8_t x, uint8_t y, uint8_t cols, uint8_t rows, const uint8_t *image) {

}