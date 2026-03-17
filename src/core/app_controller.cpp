#include <TFT_eSPI.h>
#include <modules/app_controller.h>
#include <modules/app_state.h>
#include <modules/constants.h>
#include <modules/display_setup.h>
#include <modules/hal/brightness_control.h>
#include <modules/modes/bitcoin_mode.h>
#include <modules/modes/clock_mode.h>
#include <modules/modes/weather_mode.h>
#include <modules/sprites.h>
#include <modules/variables.h>
#include <modules/wifi_connect.h>
#include <stdlib.h>
#include <time.h>

#ifndef APP_TIMEZONE
#define APP_TIMEZONE ""
#endif

namespace
{

constexpr int MODE_COUNT = 3;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 250UL;
constexpr unsigned long SERIAL_BAUD_RATE = 115200UL;
constexpr const char* NTP_SERVER = "pool.ntp.org";

bool previousButtonUpState = false;
bool previousButtonDownState = false;
unsigned long lastButtonSwitchMillis = 0;

void update_mode_variables()
{
    changed_mode = false;
    lastModeUpdate = currentMillis;
}

void handle_mode_switch_buttons()
{
    const bool buttonUpPressed = digitalRead(BUTTON_UP) == LOW;
    const bool buttonDownPressed = digitalRead(BUTTON_DOWN) == LOW;

    if (currentMillis - lastButtonSwitchMillis < BUTTON_DEBOUNCE_MS)
    {
        previousButtonUpState = buttonUpPressed;
        previousButtonDownState = buttonDownPressed;
        return;
    }

    if (buttonUpPressed && !previousButtonUpState)
    {
        mode = static_cast<DisplayMode>((static_cast<int>(mode) + 1) % MODE_COUNT);
        changed_mode = true;
        tft.fillScreen(TFT_BLACK);
        lastButtonSwitchMillis = currentMillis;
    }

    if (buttonDownPressed && !previousButtonDownState)
    {
        mode = static_cast<DisplayMode>((static_cast<int>(mode) - 1 + MODE_COUNT) % MODE_COUNT);
        changed_mode = true;
        tft.fillScreen(TFT_BLACK);
        lastButtonSwitchMillis = currentMillis;
    }

    previousButtonUpState = buttonUpPressed;
    previousButtonDownState = buttonDownPressed;
}

void render_active_mode()
{
    // Each mode has its own cadence to keep UI responsive while avoiding unnecessary redraws.
    switch (mode)
    {
        case DisplayMode::Bitcoin:
            bitcoin_logo_rotation();
            if (currentMillis - lastModeUpdate >= MODE0_UPDATE_INTERVAL || changed_mode)
            {
                bitcoin_render();
                update_mode_variables();
            }
            break;
        case DisplayMode::Clock:
            if (currentMillis - lastModeUpdate >= MODE1_UPDATE_INTERVAL || changed_mode)
            {
                if (changed_mode)
                {
                    clock_update_time();
                }
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
    // ESP32 helper that applies TZ rules and starts NTP sync in one call.
    configTzTime(APP_TIMEZONE, NTP_SERVER);
    bitcoin_update_price();
    clock_update_time();
    adjust_brightness();
    bitcoin_sync_chart();
}

void app_loop()
{
    currentMillis = millis();
    handle_mode_switch_buttons();

    // Data updates run independently from mode rendering, so mode switches stay snappy.
    if (currentMillis - lastPriceUpdate >= PRICE_UPDATE_INTERVAL)
    {
        bitcoin_update_price();
    }

    if (currentMillis - lastTimeUpdate >= TIME_UPDATE_INTERVAL)
    {
        clock_update_time();
        bitcoin_sync_chart();
    }

    // Apply smooth non-blocking brightness transitions continuously.
    adjust_brightness();

    render_active_mode();
}
