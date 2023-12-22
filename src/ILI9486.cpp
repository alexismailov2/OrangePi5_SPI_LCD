#include <spi_lcd/ILI9486.hpp>

#include "ILI9486_Commands.hpp"

#include <cstdint>
#include <cstdlib>
#include <thread>

#define TFT_MAX_PIXELS_AT_ONCE  32

namespace {
uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}
} // anonymous

#if 0

#define bmpRead32(d,o) (d[o] | (uint16_t)(d[(o)+1]) << 8 | (uint32_t)(d[(o)+2]) << 16 | (uint32_t)(d[(o)+3]) << 24)
#define bmpRead16(d,o) (d[o] | (uint16_t)(d[(o)+1]) << 8)

#define bmpColor8(c)       (((uint16_t)(((uint8_t*)(c))[0] & 0xE0) << 8) | ((uint16_t)(((uint8_t*)(c))[0] & 0x1C) << 6) | ((((uint8_t*)(c))[0] & 0x3) << 3))
#define bmpColor16(c)      ((((uint8_t*)(c))[0] | ((uint16_t)((uint8_t*)(c))[1]) << 8))
#define bmpColor24(c)      (((uint16_t)(((uint8_t*)(c))[2] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)(c))[1] & 0xFC) << 3) | ((((uint8_t*)(c))[0] & 0xF8) >> 3))
#define bmpColor32(c)      (((uint16_t)(((uint8_t*)(c))[3] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)(c))[2] & 0xFC) << 3) | ((((uint8_t*)(c))[1] & 0xF8) >> 3))

void TFT::bmpSkipPixels(fs::File &file, uint8_t bitsPerPixel, size_t len){
  size_t bytesToSkip = (len * bitsPerPixel) / 8;
  file.seek(bytesToSkip, SeekCur);
}

void TFT::bmpAddPixels(fs::File &file, uint8_t bitsPerPixel, size_t len){
  size_t bytesPerTransaction = bitsPerPixel * 4;
  uint8_t transBuf[bytesPerTransaction];
  uint16_t pixBuf[32];
  uint8_t * tBuf;
  uint8_t pixIndex = 0;
  size_t wIndex = 0, pixNow, bytesNow;
  while(wIndex < len){
    pixNow = len - wIndex;
    if(pixNow > 32){
      pixNow = 32;
    }
    bytesNow = (pixNow * bitsPerPixel) / 8;
    file.read(transBuf, bytesNow);
    tBuf = transBuf;

    for(pixIndex=0; pixIndex < pixNow; pixIndex++){
      if(bitsPerPixel == 32){
        pixBuf[pixIndex] = (bmpColor32(tBuf));
        tBuf+=4;
      } else if(bitsPerPixel == 24){
        pixBuf[pixIndex] = (bmpColor24(tBuf));
        tBuf+=3;
      } else if(bitsPerPixel == 16){
        pixBuf[pixIndex] = (bmpColor16(tBuf));
        tBuf+=2;
      } else if(bitsPerPixel == 8){
        pixBuf[pixIndex] = (bmpColor8(tBuf));
        tBuf+=1;
      } else if(bitsPerPixel == 4){
        uint16_t g = tBuf[0] & 0xF;
        if(pixIndex & 1){
          tBuf+=1;
        } else {
          g = tBuf[0] >> 4;
        }
        pixBuf[pixIndex] = ((g << 12) | (g << 7) | (g << 1));
      }
    }
    startWrite();
    writePixels(pixBuf, pixNow);
    endWrite();
    wIndex += pixNow;
  }
}

boolean TFT::drawBmpFile(fs::FS &fs, const char * path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight, uint16_t offX, uint16_t offY){
  if((x + maxWidth) > width() || (y + maxHeight) > height()){
    log_e("Bad dimensions given");
    return false;
  }

  if(!maxWidth){
    maxWidth = width() - x;
  }
  if(!maxHeight){
    maxHeight = height() - y;
  }
  //log_e("maxWidth=%i, maxHeight=%i", maxWidth, maxHeight);
  File file = fs.open(path);
  if(!file){
    if(tft_info) tft_info("Failed to open file for reading\n");
    return false;
  }
  size_t headerLen = 0x22;
  size_t fileSize = file.size();
  uint8_t headerBuf[headerLen];
  if(fileSize < headerLen || file.read(headerBuf, headerLen) < headerLen){
    log_e("Failed to read the file's header");
    file.close();
    return false;
  }

  if(headerBuf[0] != 'B' || headerBuf[1] != 'M'){
    log_e("Wrong file format");
    file.close();
    return false;
  }

  //size_t bmpSize = bmpRead32(headerBuf, 0x02);
  uint32_t dataOffset = bmpRead32(headerBuf, 0x0A);
  int32_t bmpWidthI = bmpRead32(headerBuf, 0x12);
  int32_t bmpHeightI = bmpRead32(headerBuf, 0x16);
  uint16_t bitsPerPixel = bmpRead16(headerBuf, 0x1C);

  size_t bmpWidth = abs(bmpWidthI);
  size_t bmpHeight = abs(bmpHeightI);

  if(offX >= bmpWidth || offY >= bmpHeight){
    log_e("Offset Outside of bitmap size");
    file.close();
    return false;
  }

  size_t bmpMaxWidth = bmpWidth - offX;
  size_t bmpMaxHeight = bmpHeight - offY;
  size_t outWidth = (bmpMaxWidth > maxWidth)?maxWidth:bmpMaxWidth;
  size_t outHeight = (bmpMaxHeight > maxHeight)?maxHeight:bmpMaxHeight;
  size_t ovfWidth = bmpMaxWidth - outWidth;
  size_t ovfHeight = bmpMaxHeight - outHeight;

  file.seek(dataOffset);
  startBitmap(x, y, outWidth, outHeight);

  if(ovfHeight){
    bmpSkipPixels(file, bitsPerPixel, ovfHeight * bmpWidth);
  }
  if(!offX && !ovfWidth){
    bmpAddPixels(file, bitsPerPixel, outWidth * outHeight);
  } else {
    size_t ih;
    for(ih=0;ih<outHeight;ih++){
      if(offX){
        bmpSkipPixels(file, bitsPerPixel, offX);
      }
      bmpAddPixels(file, bitsPerPixel, outWidth);
      if(ovfWidth){
        bmpSkipPixels(file, bitsPerPixel, ovfWidth);
      }
    }
  }

  endBitmap();
  file.close();
  return true;
}

#endif

ILI9486::ILI9486(ISPI &spi)
  : _spi{spi}
{
  _spi.chipSelect(true);
  //Driving ability Setting
  writeCommand(0x11); // Sleep out, also SW reset
  std::this_thread::sleep_for(std::chrono::milliseconds(120));
  //delay(120);

  writeCommand(0x3A); // Interface Pixel Format
  _spi.exchange(0x55);

  writeCommand(0xC2); // Power Control 3 (For Normal Mode)
  _spi.exchange(0x44);

  writeCommand(0xC5); // VCOM Control
  _spi.exchange(0x00);
  _spi.exchange(0x00);
  _spi.exchange(0x00);
  _spi.exchange(0x00);

  writeCommand(0xE0); // PGAMCTRL(Positive Gamma Control)
  _spi.exchange(0x0F);
  _spi.exchange(0x1F);
  _spi.exchange(0x1C);
  _spi.exchange(0x0C);
  _spi.exchange(0x0F);
  _spi.exchange(0x08);
  _spi.exchange(0x48);
  _spi.exchange(0x98);
  _spi.exchange(0x37);
  _spi.exchange(0x0A);
  _spi.exchange(0x13);
  _spi.exchange(0x04);
  _spi.exchange(0x11);
  _spi.exchange(0x0D);
  _spi.exchange(0x00);

  writeCommand(0xE1); // NGAMCTRL (Negative Gamma Correction)
  _spi.exchange(0x0F);
  _spi.exchange(0x32);
  _spi.exchange(0x2E);
  _spi.exchange(0x0B);
  _spi.exchange(0x0D);
  _spi.exchange(0x05);
  _spi.exchange(0x47);
  _spi.exchange(0x75);
  _spi.exchange(0x37);
  _spi.exchange(0x06);
  _spi.exchange(0x10);
  _spi.exchange(0x03);
  _spi.exchange(0x24);
  _spi.exchange(0x20);
  _spi.exchange(0x00);

  writeCommand(0x20); // Display Inversion OFF   RPi LCD (A)
  //      writeCommand(0x21); // Display Inversion ON    RPi LCD (B)

  writeCommand(0x36); // Memory Access Control
  _spi.exchange(0x48);

  writeCommand(0x29); // Display ON
  //delay(150);
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  _spi.chipSelect(false);
}

void ILI9486::writeCommand(uint16_t cmd)
{
  _spi.commandSelect(true);
  _spi.exchange(cmd);
  _spi.commandSelect(false);
}

void ILI9486::writeData(uint16_t dat)
{
  _spi.exchange(dat);
}

void ILI9486::setRotation(uint8_t m)
{
  _rotation = m % 4; // can't be higher than 3
  _spi.chipSelect(true);
  writeCommand(ILI9486_MADCTL);

  switch (_rotation) {
    case 0:
      _spi.exchange(MADCTL_MX |MADCTL_BGR);
      _width  = TFT_WIDTH;
      _height = TFT_HEIGHT;
      break;
    case 1:
      _spi.exchange(MADCTL_MV | MADCTL_BGR);
      _width  = TFT_HEIGHT;
      _height = TFT_WIDTH;
      break;
    case 2:
      _spi.exchange(MADCTL_MY | MADCTL_BGR);
      _width  = TFT_WIDTH;
      _height = TFT_HEIGHT;
      break;
    case 3:
      _spi.exchange(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
      _width  = TFT_HEIGHT;
      _height = TFT_WIDTH;
      break;
  }
  _spi.chipSelect(false);
}

void ILI9486::setBrigthness(uint8_t br)
{
  _spi.chipSelect(true);
  writeCommand(ILI9486_CDBVAL);
  _spi.exchange(0);
  writeCommand(ILI9486_WDBVAL);
  _spi.exchange(128);
  _spi.chipSelect(false);
}

void ILI9486::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  writeCommand(ILI9486_CASET); // Column addr set
  _spi.exchange(x >> 8);
  _spi.exchange(x & 0xFF);     // XSTART
  w=x+w-1;
  _spi.exchange(w >> 8);
  _spi.exchange(w & 0xFF);     // XEND

  writeCommand(ILI9486_PASET); // Row addr set
  _spi.exchange(y >> 8);
  _spi.exchange(y & 0xFF);     // YSTART
  h=y+h-1;
  _spi.exchange(h >> 8);
  _spi.exchange(h & 0xFF);     // YEND
}

void ILI9486::startBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  _spi.chipSelect(true);
  writeCommand(ILI9486_MADCTL);
  if(_rotation==0){_spi.exchange(MADCTL_MX | MADCTL_MY | MADCTL_ML | MADCTL_BGR);}
  if(_rotation==1){_spi.exchange(MADCTL_MH | MADCTL_MV | MADCTL_MX | MADCTL_BGR);}
  if(_rotation==2){_spi.exchange(MADCTL_MH | MADCTL_BGR);}
  if(_rotation==3){_spi.exchange(MADCTL_MV | MADCTL_MY | MADCTL_BGR);}

  setAddrWindow(x, _height - y - h ,w ,h);
  writeCommand(ILI9486_RAMWR);
  _spi.chipSelect(false);
}

void ILI9486::endBitmap()
{
  setRotation(_rotation);
}

void ILI9486::pushColor(uint16_t color)
{
  _spi.chipSelect(true);
  _spi.exchange(color);
  _spi.chipSelect(false);
}

void ILI9486::writePixel(uint16_t color)
{
  _spi.exchange(color);
}

void ILI9486::writePixels(uint16_t* colors, uint32_t len)
{
  _spi.sendBuff((uint8_t*)colors, len*2);
}

void ILI9486::writeColor(uint16_t color, uint32_t len)
{
  static uint16_t temp[TFT_MAX_PIXELS_AT_ONCE];
  size_t blen = (len > TFT_MAX_PIXELS_AT_ONCE)?TFT_MAX_PIXELS_AT_ONCE:len;
  uint16_t tlen = 0;

  for (uint32_t t=0; t<blen; t++){
    temp[t] = color;
  }

  while(len){
    tlen = (len>blen)?blen:len;
    writePixels(temp, tlen);
    len -= tlen;
  }
}

void ILI9486::writePixel(int16_t x, int16_t y, uint16_t color)
{
  if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;
  setAddrWindow(x,y,1,1);
  writeCommand(ILI9486_RAMWR);
  writePixel(color);
}

void ILI9486::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  if((x >= _width) || (y >= _height)) return;

  int16_t x2 = x + w - 1, y2 = y + h - 1;
  if((x2 < 0) || (y2 < 0)) return;

  // Clip left/top
  if(x < 0) {
    x = 0;
    w = x2 + 1;
  }
  if(y < 0) {
    y = 0;
    h = y2 + 1;
  }

  // Clip right/bottom
  if(x2 >= _width)  w = _width  - x;
  if(y2 >= _height) h = _height - y;

  int32_t len = (int32_t)w * h;
  setAddrWindow(x, y, w, h);
  writeCommand(ILI9486_RAMWR);
  writeColor(color, len);
}

void ILI9486::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  writeFillRect(x, y, 1, h, color);
}

void ILI9486::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  writeFillRect(x, y, w, 1, color);
}
void ILI9486::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  _spi.chipSelect(true);
  writePixel(x, y, color);
  _spi.chipSelect(false);
}
void ILI9486::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  _spi.chipSelect(true);
  writeFastVLine(x, y, h, color);
  _spi.chipSelect(false);
}
void ILI9486::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  _spi.chipSelect(true);
  writeFastHLine(x, y, w, color);
  _spi.chipSelect(false);
}

void ILI9486::fillScreen(uint16_t color)
{
  fillRect(0, 0, _width, _height, color);
}

void ILI9486::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  _spi.chipSelect(true);
  writeFillRect(x, y, w, h, color);
  _spi.chipSelect(false);
}

bool ILI9486::setCursor(uint16_t x, uint16_t y)
{

  if (x >= _width || y >= _height) {
    return false;
  }
  writeCommand(0x2A);
  _spi.exchange(x >> 8);
  _spi.exchange(x & 0xFF);
  writeCommand(ILI9486_RAMWR); //Column Start
  writeCommand(0x2B);
  _spi.exchange(y >> 8);
  _spi.exchange(y & 0xFF);
  writeCommand(ILI9486_RAMWR); //Row Start
  //_curX = x;
  //_curY = y;
  //_f_curPos = true;  //curPos is updated
  return true;
}
