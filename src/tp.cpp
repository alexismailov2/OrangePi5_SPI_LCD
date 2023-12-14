#include "tp.hpp"

TP::TP(uint8_t CS, uint8_t IRQ)
{
  TP_CS = CS;
  TP_IRQ = IRQ;
  pinMode(TP_CS, OUTPUT);
  digitalWrite(TP_CS, HIGH);// prevent blockage of the SPI bus
  pinMode(TP_IRQ, INPUT);
  TP_SPI = SPISettings(100000, MSBFIRST, SPI_MODE0);//slower speed
  xFaktor = float(Xmax - Xmin) / TFT_WIDTH;
  yFaktor = float(Ymax - Ymin) / TFT_HEIGHT;
  _rotation = 0;
}

uint16_t TP::TP_Send(uint8_t set_val)
{
  SPItransfer->beginTransaction(TP_SPI);// Prevent other SPI users
  digitalWrite(TP_CS, 0);
  SPItransfer->write(set_val);
  uint16_t get_val = SPItransfer->transfer16(0);
  digitalWrite(TP_CS, 1);
  SPItransfer->endTransaction();// Allow other SPI users
  return get_val >> 4;
}

void TP::loop()
{
  if (!digitalRead(TP_IRQ)) {
    read_TP(x, y);//erste Messung auslassen
    if (f_loop) {
      f_loop = false;
      if (read_TP(x, y)) {
        if (tp_pressed) tp_pressed(x, y);
      }//zweite Messung lesen
    }
  } else {
    if (f_loop == false) {
      if (tp_released) tp_released();
    }
    f_loop = true;
  }
}

void TP::setRotation(uint8_t m)
{
  _rotation = m;
}

bool TP::read_TP(uint16_t &x, uint16_t &y)
{
  uint16_t _y[3];
  uint16_t _x[3];
  uint16_t tmpxy;

  uint8_t i;
  if (digitalRead(TP_IRQ)) return false;
  for (i = 0; i < 3; i++) {
    x = TP_Send(0xD0);//x

    if ((x < Xmin) || (x > Xmax)) return false;//außerhalb des Displays
    x = Xmax - x;
    _x[i] = x / xFaktor;

    y = TP_Send(0x90);                         //y
    if ((y < Ymin) || (y > Ymax)) return false;//außerhalb des Displays
    y = Ymax - y;
    _y[i] = y / yFaktor;
  }
  x = (_x[0] + _x[1] + _x[2]) / 3;// Mittelwert bilden
  y = (_y[0] + _y[1] + _y[2]) / 3;

  //log_i("TP X org=%i",x);
  //log_i("TP Y org=%i",y);

  if (_rotation == 1) {
    tmpxy = x;
    x = y;
    y = TFT_WIDTH - tmpxy;
    if (x > TFT_HEIGHT - 1) x = 0;
    if (y > TFT_WIDTH - 1) y = 0;
  }
  if (_rotation == 2) {
    x = TFT_WIDTH - x;
    y = TFT_HEIGHT - y;
    if (x > TFT_WIDTH - 1) x = 0;
    if (y > TFT_HEIGHT - 1) y = 0;
  }
  if (_rotation == 3) {
    tmpxy = y;
    y = x;
    x = TFT_HEIGHT - tmpxy;
    if (x > TFT_HEIGHT - 1) x = 0;
    if (y > TFT_WIDTH - 1) y = 0;
  }
  //log_i("TP X=%i",x);
  //log_i("TP Y=%i",y);
  return true;
}