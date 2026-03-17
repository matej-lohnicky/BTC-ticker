#include <TFT_eSPI.h>
#include <modules/app_state.h>
#include <modules/constants.h>
#include <modules/display_setup.h>
#include <modules/sprites.h>

namespace
{

constexpr int LCD_BACKLIGHT_PIN = 38;
constexpr int DISPLAY_ROTATION = 1;
constexpr int DEFAULT_TEXT_SIZE = 2;

constexpr int BACK_LOGO_WIDTH = 74;
constexpr int BACK_LOGO_HEIGHT = 74;
constexpr int BTC_LOGO_WIDTH = 73;
constexpr int BTC_LOGO_HEIGHT = 73;
constexpr int BTC_PRICE_WIDTH = 205;
constexpr int BTC_PRICE_HEIGHT = 45;
constexpr int CHART_WIDTH = 173;
constexpr int CHART_HEIGHT = 100;
constexpr int SESSIONS_WIDTH = 32;
constexpr int SESSIONS_HEIGHT = 100;
constexpr int BTC_PERCENTS_WIDTH = 115;
constexpr int BTC_PERCENTS_HEIGHT = 86;
constexpr int CLOCK_DISPLAY_WIDTH = 320;
constexpr int CLOCK_DISPLAY_HEIGHT = 170;
constexpr int WEATHER_ICON_WIDTH = 100;
constexpr int WEATHER_ICON_HEIGHT = 85;
constexpr int TEMPERATURE_WIDTH = 140;
constexpr int TEMPERATURE_HEIGHT = 85;
constexpr int TEMPERATURE_EXTREMES_WIDTH = 66;
constexpr int TEMPERATURE_EXTREMES_HEIGHT = 85;
constexpr int TEMPERATURE_RANGE_WIDTH = 14;
constexpr int TEMPERATURE_RANGE_HEIGHT = 85;
constexpr int RAIN_CHART_WIDTH = 320;
constexpr int RAIN_CHART_HEIGHT = 85;

}  // namespace

void initialize_display()
{
    tft.begin();
    tft.setRotation(DISPLAY_ROTATION);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(DEFAULT_TEXT_SIZE);
    analogWrite(LCD_BACKLIGHT_PIN, currentBrightness);
}

void initialize_input()
{
    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
}

void initialize_sprites()
{
    back_logo.createSprite(BACK_LOGO_WIDTH, BACK_LOGO_HEIGHT);
    btc_logo.createSprite(BTC_LOGO_WIDTH, BTC_LOGO_HEIGHT);
    btc_price.createSprite(BTC_PRICE_WIDTH, BTC_PRICE_HEIGHT);
    chart.createSprite(CHART_WIDTH, CHART_HEIGHT);
    sessions.createSprite(SESSIONS_WIDTH, SESSIONS_HEIGHT);
    btc_percents.createSprite(BTC_PERCENTS_WIDTH, BTC_PERCENTS_HEIGHT);
    clock_display.createSprite(CLOCK_DISPLAY_WIDTH, CLOCK_DISPLAY_HEIGHT);
    weatherIcon.createSprite(WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT);
    temperature.createSprite(TEMPERATURE_WIDTH, TEMPERATURE_HEIGHT);
    temperatureExtremes.createSprite(TEMPERATURE_EXTREMES_WIDTH, TEMPERATURE_EXTREMES_HEIGHT);
    temperatureRange.createSprite(TEMPERATURE_RANGE_WIDTH, TEMPERATURE_RANGE_HEIGHT);
    rainChart.createSprite(RAIN_CHART_WIDTH, RAIN_CHART_HEIGHT);
}

void configure_sprite_swap_bytes()
{
    btc_logo.setSwapBytes(true);
    sessions.setSwapBytes(true);
    btc_percents.setSwapBytes(true);
    weatherIcon.setSwapBytes(true);
    temperatureExtremes.setSwapBytes(true);
}
