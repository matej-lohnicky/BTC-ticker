#pragma once

#include <stddef.h>
#include <stdint.h>

enum class ScreenshotScreen : uint8_t
{
    WifiChoose = 1,
    Keyboard = 2,
    Bitcoin = 3,
    Weather = 4,
    Clock = 5,
    Complete = 255,
};

void screenshot_send_rgb565_frame(ScreenshotScreen screen, uint16_t width, uint16_t height,
                                  const uint16_t* buffer);
void screenshot_send_complete();
