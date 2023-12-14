#pragma once

/**
 *
 * x,y | Ux,Uy  0  ,0     | 1922,1930
 * x,y | Ux,Uy  320,0     |  140,1930
 * x,y | Ux,Uy  0  ,480   | 1922,125
 * x,y | Ux,Uy  320,480   |  140,125
 * x: (1922-140)/320 = 5,5687mV pro Pixel
 * y: (1930-125)/480 = 3,7604mV pro Pixel
 */
class TP
{
public:
  TP(uint8_t TP_CS, uint8_t TP_IRQ);
  void loop();
  void setRotation(uint8_t m);

protected:
  uint16_t TP_Send(uint8_t set_val);
  bool read_TP(uint16_t &x, uint16_t &y);

private:
  SPISettings TP_SPI;
  uint8_t TP_CS, TP_IRQ;
  uint16_t x = 0, y = 0;
  uint8_t _rotation;
  boolean f_loop = false;
  //const uint8_t TP_Dummy=0x80; //nur Startbit für XPT2046
  float xFaktor;
  float yFaktor;
  const uint16_t Xmax = 1922;
  const uint16_t Xmin = 140;
  const uint16_t Ymax = 1930;
  const uint16_t Ymin = 125;
};