#pragma once

constexpr int BUTTON_DOWN = 0;
constexpr int BUTTON_UP = 14;

using Color = unsigned short;

// graphics
constexpr int row_height = 20;
constexpr int row_name_x_padding = 10;
constexpr int row_name_y_padding = 10;

constexpr unsigned long ROTATION_INTERVAL = 15UL;  // BTC logo rotation speed

#ifndef SCREENSHOT_MODE
#define SCREENSHOT_MODE 1
#endif

constexpr unsigned long SCREENSHOT_STABILIZE_DELAY_MS = 180UL;