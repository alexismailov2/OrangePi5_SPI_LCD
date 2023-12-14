#include <spi_lcd/ISPI.hpp>
#include <spi_lcd/ILI9486.hpp>
#include <wiringPi.h>
#include <wiringPiSpi.h>
#include <iostream>
#include <thread>

class TestSPI : public ISPI {
public:
  TestSPI() {
    wiringPiSetup();
    pinMode(22, OUTPUT);
    digitalWrite(22, LOW);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    digitalWrite(22, HIGH);

    pinMode(24, OUTPUT);
    digitalWrite(24, HIGH);
    pinMode(18, OUTPUT);
    digitalWrite(18, LOW);
    if (wiringPiSPISetup(0, 10000000) == -1) {
      throw std::runtime_error("Could not initialize wiringPi");
    }
    system("sudo ./wiringOP/gpio/gpio readall");
//    spi_TFT->begin(sclk, miso, mosi, -1);
//    spi_TFT->setFrequency(_freq);
//
//    TFT_SPI = SPISettings(_freq, MSBFIRST, SPI_MODE0);
//
//    String info="";
//    TFT_CS = CS;
//    TFT_DC = DC;
//
//    pinMode(TFT_DC, OUTPUT);
//    digitalWrite(TFT_DC, LOW);
//    pinMode(TFT_CS, OUTPUT);
//    digitalWrite(TFT_CS, HIGH);
//
//    spi_TFT->begin(TFT_SCK, TFT_MISO, TFT_MOSI, -1);
  }
  void chipSelect(bool enable) override {
    printf("%s", enable ? "~CS " : "\nCS \n");
    digitalWrite(24, !enable ? HIGH : LOW);
  }
  void commandSelect(bool enable) override {
    printf("%s", enable ? "\nCMD " : "DAT ");
    digitalWrite(18, enable ? HIGH : LOW);
  }
  auto exchange(uint8_t data) -> uint8_t override {
    printf("0x%02X ", data);
    wiringPiSPIDataRW(0, &data, 1);
    return data;
  }
  void sendBuff(uint8_t* data, uint32_t size) override {
    for (auto i = 0; i < size; ++i) {
      printf("0x%02X ", data[i]);
    }
    printf("\n");
    wiringPiSPIDataRW(0, data, size);
  }
};

int main()
{
  auto testSpi = TestSPI();
  auto ili9486 = ILI9486(testSpi);
  for(int i = 0; i < 10; i++)
  {
    ili9486.fillScreen(0xFFFF);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ili9486.fillScreen(0x0000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  return 0;
}
