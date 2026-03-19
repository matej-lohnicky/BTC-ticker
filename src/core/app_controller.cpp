#include <TFT_eSPI.h>
#include <modules/app_controller.h>
#include <modules/app_state.h>
#include <modules/constants.h>
#include <modules/display_setup.h>
#include <modules/modes/bitcoin_mode.h>
#include <modules/modes/clock_mode.h>
#include <modules/modes/weather_mode.h>
#include <modules/screenshot.h>
#include <modules/sprites.h>

#include <vector>

namespace
{

constexpr int FULL_SCREEN_X = 0;
constexpr int FULL_SCREEN_Y = 0;
constexpr unsigned long SERIAL_BAUD_RATE = 115200UL;
constexpr unsigned long STREAM_START_DELAY_MS = 2200UL;
constexpr unsigned long SCREEN_CLEAR_DELAY_MS = 350UL;
constexpr unsigned long SCREEN_RENDER_SETTLE_MS = 650UL;
constexpr unsigned long SCREEN_CAPTURE_GAP_MS = 450UL;
constexpr int SCREEN_W = 320;
constexpr int SCREEN_H = 170;

void capture_current_screen(ScreenshotScreen screen, TFT_eSprite& frame)
{
    delay(SCREENSHOT_STABILIZE_DELAY_MS + SCREEN_RENDER_SETTLE_MS);
    frame.pushSprite(FULL_SCREEN_X, FULL_SCREEN_Y);
    screenshot_send_rgb565_frame(screen, static_cast<uint16_t>(frame.width()),
                                 static_cast<uint16_t>(frame.height()),
                                 static_cast<const uint16_t*>(frame.getPointer()));
    delay(SCREEN_CAPTURE_GAP_MS);
}

void clear_for_next_screen()
{
    tft.fillScreen(TFT_BLACK);
    delay(SCREEN_CLEAR_DELAY_MS);
}

void render_mock_wifi_choose_screen(TFT_eSprite& frame)
{
    constexpr int ROW_FRAME_TOP_OFFSET = 3;
    constexpr int ROW_FRAME_WIDTH = 319;
    constexpr int SELECTED_ROW_INDEX = 0;
    constexpr const char* networks[] = {"SatoshiNet", "NODE-5G", "HALVING-CLUB", "Cafe_WiFi"};

    frame.fillSprite(TFT_BLACK);
    frame.setTextColor(TFT_WHITE, TFT_BLACK);
    frame.setTextSize(2);
    int y = row_name_y_padding;
    for (int i = 0; i < 4; ++i)
    {
        frame.setCursor(row_name_x_padding, y);
        frame.print(networks[i]);
        y += row_height;
    }

    const int selectedY = row_name_y_padding + SELECTED_ROW_INDEX * row_height;
    frame.drawRect(0, selectedY - ROW_FRAME_TOP_OFFSET, ROW_FRAME_WIDTH, row_height + 1, TFT_BLUE);
}

void render_mock_keyboard_screen(TFT_eSprite& frame)
{
    constexpr int GRID_COLS = 10;
    constexpr int GRID_ROWS = 4;
    constexpr int CELL = 32;
    constexpr int GRID_TOP_Y = 42;
    constexpr int GRID_TEXT_OFFSET_X = 12;
    constexpr int GRID_TEXT_OFFSET_Y = 9;
    constexpr int INPUT_TEXT_X = 5;
    constexpr int INPUT_TEXT_Y = 5;
    constexpr int SELECTED_COL = 0;
    constexpr int SELECTED_ROW = 1;
    const char* keys = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ.+";

    frame.fillSprite(TFT_BLACK);
    frame.setTextColor(TFT_WHITE, TFT_BLACK);
    frame.setTextSize(2);
    frame.setCursor(INPUT_TEXT_X, INPUT_TEXT_Y);
    frame.print("pass:satoshi2026");

    for (int c = 0; c <= GRID_COLS; ++c)
    {
        frame.drawLine(c * CELL, GRID_TOP_Y, c * CELL, GRID_TOP_Y + GRID_ROWS * CELL, TFT_WHITE);
    }
    for (int r = 0; r <= GRID_ROWS; ++r)
    {
        frame.drawLine(0, GRID_TOP_Y + r * CELL, SCREEN_W, GRID_TOP_Y + r * CELL, TFT_WHITE);
    }

    int idx = 0;
    for (int r = 0; r < GRID_ROWS; ++r)
    {
        for (int c = 0; c < GRID_COLS; ++c)
        {
            if (idx < 38)
            {
                frame.setCursor(GRID_TEXT_OFFSET_X + c * CELL,
                                GRID_TOP_Y + GRID_TEXT_OFFSET_Y + r * CELL);
                frame.print(String(keys[idx]));
            }
            idx++;
        }
    }

    frame.fillRect(289, 140, 30, 29, TFT_BLACK);
    frame.fillRect(257, 140, 30, 29, TFT_BLACK);
    frame.fillRect(273, 154, 10, 1, TFT_WHITE);
    frame.fillTriangle(265, 154, 272, 150, 272, 158, TFT_WHITE);
    frame.fillRect(301, 155, 7, 8, TFT_WHITE);
    frame.fillTriangle(296, 155, 304, 147, 312, 155, TFT_WHITE);

    frame.drawRect(SELECTED_COL * CELL, GRID_TOP_Y + SELECTED_ROW * CELL, CELL + 1, CELL + 1,
                   TFT_BLUE);
}

void render_mock_bitcoin_screen(TFT_eSprite& frame)
{
    static const int chartPoints[] = {96200,  97050,  98020,  98880,  99710,
                                      100340, 100910, 101770, 102960, 103456};

    changed_mode = true;
    globalHours = 15;
    globalMinutes = 45;
    globalDay = 4;
    chartTime = globalHours;
    price = 103456;
    percentChange = 3.87;
    readings.clear();
    for (int v : chartPoints)
    {
        readings.push_back(v);
    }

    tft.fillScreen(TFT_BLACK);
    bitcoin_logo_rotation();
    bitcoin_render();

    frame.fillSprite(TFT_BLACK);
    back_logo.pushToSprite(&frame, 20, 10);
    btc_price.pushToSprite(&frame, 118, 15);
    btc_percents.pushToSprite(&frame, 0, 85);
    chart.pushToSprite(&frame, 115, 70);
    sessions.pushToSprite(&frame, 288, 70);
}

void render_mock_weather_screen(TFT_eSprite& frame)
{
    static const int rainProfile[] = {5, 5, 5, 6, 8,  10, 12, 13, 14, 12, 10, 8,
                                      7, 6, 6, 8, 10, 12, 15, 13, 11, 9,  7,  6};

    globalHours = 14;
    globalMinutes = 20;
    weatherCode = 0;
    currentTemperature = 23.8F;
    sunriseHours = 6;
    sunriseMinutes = 21;
    sunsetHours = 18;
    sunsetMinutes = 44;
    precipitationProbability.clear();
    for (int v : rainProfile)
    {
        precipitationProbability.push_back(v);
    }

    tft.fillScreen(TFT_BLACK);
    weather_render();

    frame.fillSprite(TFT_BLACK);
    weatherIcon.pushToSprite(&frame, 0, 0);
    temperature.pushToSprite(&frame, 100, 0);
    temperatureExtremes.pushToSprite(&frame, 240, 0);
    temperatureRange.pushToSprite(&frame, 305, 0);
    rainChart.pushToSprite(&frame, 0, 85);
}

void render_mock_clock_screen(TFT_eSprite& frame)
{
    globalHours = 9;
    globalMinutes = 33;
    globalDay = 4;

    tft.fillScreen(TFT_BLACK);
    clock_render();

    frame.fillSprite(TFT_BLACK);
    clock_display.pushToSprite(&frame, 0, 0);
}

void run_screenshot_sequence()
{
    TFT_eSprite frame(&tft);
    frame.setColorDepth(16);
    if (!frame.createSprite(SCREEN_W, SCREEN_H))
    {
        tft.fillScreen(TFT_RED);
        return;
    }

    clear_for_next_screen();
    render_mock_wifi_choose_screen(frame);
    capture_current_screen(ScreenshotScreen::WifiChoose, frame);

    clear_for_next_screen();
    render_mock_keyboard_screen(frame);
    capture_current_screen(ScreenshotScreen::Keyboard, frame);

    clear_for_next_screen();
    render_mock_bitcoin_screen(frame);
    capture_current_screen(ScreenshotScreen::Bitcoin, frame);

    clear_for_next_screen();
    render_mock_weather_screen(frame);
    capture_current_screen(ScreenshotScreen::Weather, frame);

    clear_for_next_screen();
    render_mock_clock_screen(frame);
    capture_current_screen(ScreenshotScreen::Clock, frame);

    screenshot_send_complete();
    frame.deleteSprite();
}

}  // namespace

void app_setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    initialize_display();
    initialize_input();
    initialize_sprites();
    configure_sprite_swap_bytes();
    delay(STREAM_START_DELAY_MS);
    run_screenshot_sequence();
}

void app_loop() { delay(1000); }
