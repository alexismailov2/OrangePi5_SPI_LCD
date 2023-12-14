#pragma once

#include <cstdint>

struct ISPI
{
  virtual void chipSelect(bool enable) = 0;
  virtual void commandSelect(bool enable) {};
  virtual auto exchange(uint8_t data) -> uint8_t = 0;
  virtual void sendBuff(uint8_t* data, uint32_t size) = 0;
};
