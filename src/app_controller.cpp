#include <TFT_eSPI.h>
#include <modules/app_controller.h>
#include <modules/app_state.h>
#include <modules/bitcoin_mode.h>
#include <modules/clock_mode.h>
#include <modules/constants.h>
#include <modules/display_setup.h>
#include <modules/sprites.h>
#include <modules/variables.h>
#include <modules/weather_mode.h>
#include <modules/wifi_connect.h>
#include <time.h>

namespace
{

constexpr int MODE_COUNT = 3;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 250UL;
constexpr unsigned long SERIAL_BAUD_RATE = 115200UL;
constexpr const char* NTP_SERVER = "pool.ntp.org";
constexpr long GMT_OFFSET_SEC = 3600;
constexpr int DAYLIGHT_OFFSET_SEC = 3600;

void update_mode_variables()
{
    changed_mode = false;
    lastModeUpdate = currentMillis;
}

void handle_mode_switch_buttons()
{
    if (digitalRead(BUTTON_UP) == LOW)
    {
        mode = static_cast<DisplayMode>((static_cast<int>(mode) + 1) % MODE_COUNT);
        changed_mode = true;
        tft.fillScreen(TFT_BLACK);
        delay(BUTTON_DEBOUNCE_MS);
    }

    if (digitalRead(BUTTON_DOWN) == LOW)
    {
        mode = static_cast<DisplayMode>((static_cast<int>(mode) - 1 + MODE_COUNT) % MODE_COUNT);
        changed_mode = true;
        tft.fillScreen(TFT_BLACK);
        delay(BUTTON_DEBOUNCE_MS);
    }
}

void render_active_mode()
{
    switch (mode)
    {
        case DisplayMode::Bitcoin:
            bitcoin_logo_rotation();
            if (currentMillis - lastModeUpdate >=
                    static_cast<unsigned long>(mode0_update_interval) ||
                changed_mode)
            {
                bitcoin_render();
                update_mode_variables();
            }
            break;
        case DisplayMode::Clock:
            if (currentMillis - lastModeUpdate >= MODE1_UPDATE_INTERVAL || changed_mode)
            {
                clock_render();
                update_mode_variables();
            }
            break;
        case DisplayMode::Weather:
            if (currentMillis - lastModeUpdate >= MODE2_UPDATE_INTERVAL || changed_mode)
            {
                weather_render();
                update_mode_variables();
            }
            break;
    }
}

}  // namespace

void app_setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    initialize_display();
    initialize_input();
    initialize_sprites();
    configure_sprite_swap_bytes();

    connect_wifi();
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    bitcoin_update_price();
    clock_update_time();
    adjust_brightness();
    bitcoin_sync_chart();
}

void app_loop()
{
    currentMillis = millis();
    handle_mode_switch_buttons();

    if (currentMillis - lastPriceUpdate >= PRICE_UPDATE_INTERVAL)
    {
        bitcoin_update_price();
    }

    if (currentMillis - lastTimeUpdate >= TIME_UPDATE_INTERVAL)
    {
        clock_update_time();
        adjust_brightness();
        bitcoin_sync_chart();
    }

    render_active_mode();
}
