#include <Arduino.h>
#include <modules/app_state.h>
#include <modules/hal/brightness_control.h>

namespace
{

constexpr byte DAY_BRIGHTNESS = 250;
constexpr byte EVENING_BRIGHTNESS = 50;
constexpr byte NIGHT_BRIGHTNESS = 5;
constexpr int LCD_BACKLIGHT_PIN = 38;
constexpr unsigned long BRIGHTNESS_STEP_DELAY_MS = 50UL;
unsigned long lastBrightnessStepMillis = 0;

}  // namespace

void adjust_brightness()
{
    const unsigned long now = millis();
    if (now - lastBrightnessStepMillis < BRIGHTNESS_STEP_DELAY_MS)
    {
        return;
    }

    const int currentTotalMinutes = globalHours * 60 + globalMinutes;
    const int sunsetTotalMinutes = sunsetHours * 60 + sunsetMinutes;
    byte newBrightness = currentBrightness;

    switch (globalHours)
    {
        case 6 ... 17:
            newBrightness = DAY_BRIGHTNESS;
            break;
        case 18 ... 21:
            newBrightness =
                (currentTotalMinutes < sunsetTotalMinutes) ? DAY_BRIGHTNESS : EVENING_BRIGHTNESS;
            break;
        case 22 ... 23:
        case 0 ... 5:
            newBrightness = NIGHT_BRIGHTNESS;
            break;
    }

    if (newBrightness > currentBrightness)
    {
        ++currentBrightness;
        analogWrite(LCD_BACKLIGHT_PIN, currentBrightness);
        lastBrightnessStepMillis = now;
    }
    else if (newBrightness < currentBrightness)
    {
        --currentBrightness;
        analogWrite(LCD_BACKLIGHT_PIN, currentBrightness);
        lastBrightnessStepMillis = now;
    }
}