#ifndef MIN_1306
  #define MIN_1306

  #define BLACK 0
  #define WHITE 1
  #define XOR 2

  #include <Arduino.h>

  void initDisplay ();
  void clearDisplay ();
  void setFontSize (uint8_t size);
  void setFontColor (uint8_t color);
  void drawRect (uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, bool filled);
  void drawBitmap (uint8_t x, uint8_t y, uint8_t cols, uint8_t rows,const uint8_t *image);
#endif