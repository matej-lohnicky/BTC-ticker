#include <TFT_eSPI.h>
#include <images/FreeSansBold55pt7b.h>
#include <modules/app_state.h>
#include <modules/clock_mode.h>
#include <modules/sprites.h>
#include <modules/variables.h>
#include <time.h>

namespace
{

constexpr byte DAY_BRIGHTNESS = 250;
constexpr byte EVENING_BRIGHTNESS = 50;
constexpr byte NIGHT_BRIGHTNESS = 5;
constexpr unsigned long OFFLINE_MINUTE_INTERVAL_MS = 60000UL;
constexpr int MAX_MINUTES = 59;
constexpr int TIME_SYNC_MAX_RETRIES = 5;
constexpr unsigned long TIME_SYNC_RETRY_DELAY_MS = 1000UL;
constexpr int LCD_BACKLIGHT_PIN = 38;
constexpr unsigned long BRIGHTNESS_STEP_DELAY_MS = 50UL;

constexpr int CLOCK_RECT_LEFT_X = 18;
constexpr int CLOCK_RECT_RIGHT_X = 177;
constexpr int CLOCK_RECT_Y = 19;
constexpr int CLOCK_RECT_SIZE = 132;
constexpr int CLOCK_RECT_RADIUS = 14;
constexpr int CLOCK_DOT_X = 158;
constexpr int CLOCK_DOT_TOP_Y = 70;
constexpr int CLOCK_DOT_BOTTOM_Y = 91;
constexpr int CLOCK_DOT_SIZE = 9;
constexpr int CLOCK_DOT_RADIUS = 1;
constexpr int CLOCK_TEXT_BASELINE_Y = 121;
constexpr int CLOCK_HOUR_1_X = 51;
constexpr int CLOCK_HOUR_2_X = 114;
constexpr int CLOCK_MINUTE_1_X = 210;
constexpr int CLOCK_MINUTE_2_X = 273;

String left_digit(int value) { return String((value / 10) % 10); }

String right_digit(int value) { return String(value % 10); }

}  // namespace

void clock_update_time()
{
    lastTimeUpdate = currentMillis;

    if (offline_mode)
    {
        if (currentMillis <= offline_minute_update + OFFLINE_MINUTE_INTERVAL_MS)
        {
            return;
        }

        globalMinutes += 1;
        if (globalMinutes > MAX_MINUTES)
        {
            globalMinutes = 0;
            globalHours += 1;
        }

        offline_minute_update = currentMillis;
        return;
    }

    struct tm timeinfo;
    int retries = 0;

    while (!getLocalTime(&timeinfo) && retries < TIME_SYNC_MAX_RETRIES)
    {
        Serial.println("Waiting for local time sync...");
        delay(TIME_SYNC_RETRY_DELAY_MS);
        ++retries;
    }

    if (retries == TIME_SYNC_MAX_RETRIES)
    {
        Serial.println("Time sync unavailable, keeping previous values");
        return;
    }

    globalMinutes = timeinfo.tm_min;
    globalHours = timeinfo.tm_hour;
    globalDay = timeinfo.tm_wday;

    chartTime = chartFasterMode ? globalMinutes : globalHours;
}

void clock_render()
{
    clock_display.setFreeFont(&FreeSansBold55pt7b);
    clock_display.setTextSize(1);
    clock_display.setTextColor(TFT_WHITE);
    clock_display.fillSprite(TFT_BLACK);

    clock_display.fillRoundRect(CLOCK_RECT_LEFT_X, CLOCK_RECT_Y, CLOCK_RECT_SIZE, CLOCK_RECT_SIZE,
                                CLOCK_RECT_RADIUS, TFT_DARKGREY);
    clock_display.fillRoundRect(CLOCK_RECT_RIGHT_X, CLOCK_RECT_Y, CLOCK_RECT_SIZE, CLOCK_RECT_SIZE,
                                CLOCK_RECT_RADIUS, TFT_DARKGREY);
    clock_display.fillRoundRect(CLOCK_DOT_X, CLOCK_DOT_TOP_Y, CLOCK_DOT_SIZE, CLOCK_DOT_SIZE,
                                CLOCK_DOT_RADIUS, TFT_LIGHTGREY);
    clock_display.fillRoundRect(CLOCK_DOT_X, CLOCK_DOT_BOTTOM_Y, CLOCK_DOT_SIZE, CLOCK_DOT_SIZE,
                                CLOCK_DOT_RADIUS, TFT_LIGHTGREY);

    String hour1 = left_digit(globalHours);
    String hour2 = right_digit(globalHours);
    String minute1 = left_digit(globalMinutes);
    String minute2 = right_digit(globalMinutes);

    clock_display.setCursor(CLOCK_HOUR_1_X - (clock_display.textWidth(hour1) / 2),
                            CLOCK_TEXT_BASELINE_Y);
    clock_display.print(hour1);
    clock_display.setCursor(CLOCK_HOUR_2_X - (clock_display.textWidth(hour2) / 2),
                            CLOCK_TEXT_BASELINE_Y);
    clock_display.print(hour2);
    clock_display.setCursor(CLOCK_MINUTE_1_X - (clock_display.textWidth(minute1) / 2),
                            CLOCK_TEXT_BASELINE_Y);
    clock_display.print(minute1);
    clock_display.setCursor(CLOCK_MINUTE_2_X - (clock_display.textWidth(minute2) / 2),
                            CLOCK_TEXT_BASELINE_Y);
    clock_display.print(minute2);

    clock_display.pushSprite(0, 0);
}

void adjust_brightness()
{
    byte newBrightness = currentBrightness;

    switch (globalHours)
    {
        case 6 ... 17:
            newBrightness = DAY_BRIGHTNESS;
            break;
        case 18 ... 21:
            newBrightness = (globalHours < sunsetHours) ? DAY_BRIGHTNESS : EVENING_BRIGHTNESS;
            break;
        case 22 ... 23:
        case 0 ... 5:
            newBrightness = NIGHT_BRIGHTNESS;
            break;
    }

    if (newBrightness > currentBrightness)
    {
        while (newBrightness != currentBrightness)
        {
            currentBrightness++;
            analogWrite(LCD_BACKLIGHT_PIN, currentBrightness);
            delay(BRIGHTNESS_STEP_DELAY_MS);
        }
    }
    else if (newBrightness < currentBrightness)
    {
        while (newBrightness != currentBrightness)
        {
            --currentBrightness;
            analogWrite(LCD_BACKLIGHT_PIN, currentBrightness);
            delay(BRIGHTNESS_STEP_DELAY_MS);
        }
    }
}
