#pragma once

constexpr int BUTTON_DOWN = 0;
constexpr int BUTTON_UP = 14;

using Color = uint16_t;

// graphics
constexpr int row_height = 20;
constexpr int row_name_x_padding = 10;
constexpr int row_name_y_padding = 10;

// intervals
constexpr unsigned long TIME_UPDATE_INTERVAL = 5000;
constexpr unsigned long PRICE_UPDATE_INTERVAl = 60000;
constexpr unsigned long MODE0_UPDATE_INTERVAL_NORMAL = 60000;  // standard price
constexpr unsigned long MODE0_UPDATE_INTERVAL_FAST = 30000;  // showcase faster price
constexpr unsigned long MODE1_UPDATE_INTERVAL = 5000;   // clock
constexpr unsigned long MODE2_UPDATE_INTERVAL = 300000;  // weather
constexpr unsigned long ROTATION_INTERVAL = 15;  // BTC logo rotation speed
constexpr int HOLD_TIME = 1000; //  time needed to hold the button