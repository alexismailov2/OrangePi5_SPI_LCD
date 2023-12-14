#pragma once

constexpr auto TFT_WIDTH = 320;
constexpr auto TFT_HEIGHT = 480;

constexpr auto ILI9486_INVOFF = 0x20; // Display Inversion OFF
constexpr auto ILI9486_INVON = 0x21;  // Display Inversion ON
constexpr auto ILI9486_CASET = 0x2A;  // Display On
constexpr auto ILI9486_PASET = 0x2B;  // Page Address Set
constexpr auto ILI9486_RAMWR = 0x2C;  // Memory Write
constexpr auto ILI9486_MADCTL = 0x36; // Memory Data Access Control
constexpr auto MADCTL_MY = 0x80;      // Bit 7 Parameter MADCTL
constexpr auto MADCTL_MX = 0x40;      // Bit 6 Parameter MADCTL
constexpr auto MADCTL_MV = 0x20;      // Bit 5 Parameter MADCTL
constexpr auto MADCTL_ML = 0x10;      // Bit 4 Parameter MADCTL
constexpr auto MADCTL_BGR = 0x08;     // Bit 3 Parameter MADCTL
constexpr auto MADCTL_MH = 0x04;      // Bit 2 Parameter MADCTL
constexpr auto ILI9486_WDBVAL = 0x51; // Write Display Brightness Value
constexpr auto ILI9486_CDBVAL = 0x53; // Write Control Display Value