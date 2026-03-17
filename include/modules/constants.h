#pragma once

#include <cstdint>

constexpr int BUTTON_DOWN = 0;
constexpr int BUTTON_UP = 14;

using Color = uint16_t;

// graphics
constexpr int row_height = 20;
constexpr int row_name_x_padding = 10;
constexpr int row_name_y_padding = 10;

// intervals
constexpr unsigned long TIME_UPDATE_INTERVAL = 5000UL;
constexpr unsigned long PRICE_UPDATE_INTERVAL = 60000UL;
constexpr unsigned long PRICE_UPDATE_INTERVAl = PRICE_UPDATE_INTERVAL;
constexpr unsigned long MODE0_UPDATE_INTERVAL_NORMAL = 60000UL;  // standard price
constexpr unsigned long MODE0_UPDATE_INTERVAL_FAST = 30000UL;    // showcase faster price
constexpr unsigned long MODE1_UPDATE_INTERVAL = 5000UL;          // clock
constexpr unsigned long MODE2_UPDATE_INTERVAL = 300000UL;        // weather
constexpr unsigned long ROTATION_INTERVAL = 15UL;                // BTC logo rotation speed
constexpr int HOLD_TIME = 1000;                                  //  time needed to hold the button