#pragma once

#include <spi_lcd/ISPI.hpp>

class ILI9486
{
public:
  ILI9486(ISPI& spi);

  void writeCommand(uint16_t cmd);

  void writeData(uint16_t dat);

  void setRotation(uint8_t m);

  void setBrigthness(uint8_t br);

  void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

  void startBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

  void endBitmap();

  void pushColor(uint16_t color);

  void writePixel(uint16_t color);

  void writePixels(uint16_t* colors, uint32_t len);

  void writeColor(uint16_t color, uint32_t len);

  void writePixel(int16_t x, int16_t y, uint16_t color);

  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

  void drawPixel(int16_t x, int16_t y, uint16_t color);

  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

  void fillScreen(uint16_t color);

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

  bool setCursor(uint16_t x, uint16_t y);

private:
  ISPI& _spi;

  uint16_t _height = 480;
  uint16_t _width = 320;
  uint8_t _rotation = 0;
};

