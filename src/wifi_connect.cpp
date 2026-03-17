#include <TFT_eSPI.h>
#include <WiFi.h>
#include <modules/app_state.h>
#include <modules/constants.h>
#include <modules/sprites.h>
#include <modules/utils.h>
#include <modules/variables.h>
#include <modules/wifi_connect.h>
#include <time.h>

#include <cstring>
#include <vector>

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

namespace
{

constexpr int WIFI_SCAN_PASSES = 3;
constexpr int STATUS_TEXT_X = 20;
constexpr int STATUS_TEXT_Y = 20;
constexpr unsigned long KEYBOARD_CONFIRM_DELAY_MS = 500UL;
constexpr int WIFI_CONNECT_ATTEMPTS = 20;
constexpr unsigned long WIFI_CONNECT_RETRY_DELAY_MS = 1000UL;
constexpr unsigned long MISSING_CREDENTIALS_MESSAGE_DELAY_MS = 1500UL;
constexpr int FAILURE_RETRY_DELAY_MS = 4;

constexpr int OFFLINE_SEED_PRICE = 68671;
constexpr double OFFLINE_SEED_PERCENT_CHANGE = 1.52;
constexpr int OFFLINE_SEED_HOUR = 9;
constexpr int OFFLINE_SEED_MINUTE = 32;
constexpr int OFFLINE_SEED_DAY = 1;
constexpr int OFFLINE_SEED_WEATHER_CODE = 1;
constexpr float OFFLINE_SEED_TEMPERATURE = 22.5F;
constexpr int OFFLINE_SUNRISE_HOUR = 4;
constexpr int OFFLINE_SUNRISE_MINUTE = 58;
constexpr int OFFLINE_SUNSET_HOUR = 20;
constexpr int OFFLINE_SUNSET_MINUTE = 44;

const std::vector<String> BOOT_MODES = {"Home", "Standard", "Showcase", "Offline"};
const std::vector<int> OFFLINE_SEED_PRECIPITATION = {0,  0,  0, 0,  0,  10, 15, 20, 40, 60, 60, 30,
                                                     20, 10, 5, 15, 20, 10, 5,  0,  0,  0,  0,  0};

void show_status_screen(const String& message)
{
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(STATUS_TEXT_X, STATUS_TEXT_Y);
    tft.print(message);
}

}  // namespace

std::vector<String> get_available_wifis()
{
    std::vector<String> result;

    for (int i = 0; i < WIFI_SCAN_PASSES; i++)
    {  // searching for networks, 3 times to ensure seeing all of them
        tft.print(".");
        int numNetworks = WiFi.scanNetworks();

        for (int i = 0; i < numNetworks; i++)
        {
            String wifiName = WiFi.SSID(i);
            if (std::find(result.begin(), result.end(), wifiName) == result.end())
            {
                result.push_back(wifiName);
            }
        }
    }

    return result;
}

String choose_wifi()
{
    show_status_screen("Loading networks");

    const std::vector<String> wifis = get_available_wifis();
    unsigned selected = row_choice(wifis);
    return wifis[selected].c_str();
}

void user_input_wifi()
{
    while (true)
    {
        String ssid = choose_wifi();
        String password = keyboard_input();
        delay(KEYBOARD_CONFIRM_DELAY_MS);  // KEEP SOME DELAY, for the buttons to work properly

        WiFi.begin(ssid, password);
        show_status_screen("Connecting to WiFi");

        for (int i = 0; WiFi.status() != WL_CONNECTED && i < WIFI_CONNECT_ATTEMPTS; ++i)
        {
            delay(WIFI_CONNECT_RETRY_DELAY_MS);
            tft.print(".");
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            tft.fillScreen(TFT_BLACK);
            return;
        }
        show_status_screen("Failed to connect to WiFi please try again");
        delay(FAILURE_RETRY_DELAY_MS);
    }
}

void fast_connect_wifi()
{
    show_status_screen("");

    if (std::strlen(WIFI_SSID) == 0 || std::strlen(WIFI_PASSWORD) == 0)
    {
        tft.print("No .env WiFi credentials found");
        delay(MISSING_CREDENTIALS_MESSAGE_DELAY_MS);
        user_input_wifi();
        return;
    }

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    tft.print("Connecting to WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_CONNECT_ATTEMPTS)
    {
        delay(WIFI_CONNECT_RETRY_DELAY_MS);
        tft.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {  // WiFi connected successfully
        tft.fillScreen(TFT_BLACK);
    }
    else
    {  // Wi-Fi connection failed
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(STATUS_TEXT_X, STATUS_TEXT_Y);
        tft.print("Failed to connect to WiFi, please try again");
        while (true)
        {
        }  // prevets further code execution without the internet
    }
}

void connect_wifi()
{
    int selected = row_choice(BOOT_MODES);

    switch (selected)
    {
        case 0:  // home, credentials from firmware
            offline_mode = false;
            fast_connect_wifi();
            chartFasterMode = false;
            mode0_update_interval = MODE0_UPDATE_INTERVAL_NORMAL;
            break;
        case 1:  // outside from home, credentials from user
            offline_mode = false;
            user_input_wifi();
            chartFasterMode = false;
            mode0_update_interval = MODE0_UPDATE_INTERVAL_NORMAL;
            break;
        case 2:  // showcase, faster
            offline_mode = false;
            user_input_wifi();
            chartFasterMode = true;
            mode0_update_interval = MODE0_UPDATE_INTERVAL_FAST;
            break;
        case 3:  // offline mode, random price generator
            WiFi.disconnect(true);
            offline_mode = true;
            chartFasterMode = true;
            mode0_update_interval = MODE0_UPDATE_INTERVAL_FAST;
            readings = {OFFLINE_SEED_PRICE};
            price = OFFLINE_SEED_PRICE;
            percentChange = OFFLINE_SEED_PERCENT_CHANGE;
            globalHours = OFFLINE_SEED_HOUR;
            globalMinutes = OFFLINE_SEED_MINUTE;
            globalDay = OFFLINE_SEED_DAY;
            offline_minute_update = millis();
            sunriseHours = OFFLINE_SUNRISE_HOUR;
            sunriseMinutes = OFFLINE_SUNRISE_MINUTE;
            sunsetHours = OFFLINE_SUNSET_HOUR;
            sunsetMinutes = OFFLINE_SUNSET_MINUTE;
            precipitationProbability = OFFLINE_SEED_PRECIPITATION;
            weatherCode = OFFLINE_SEED_WEATHER_CODE;
            currentTemperature = OFFLINE_SEED_TEMPERATURE;
            break;
    }
    tft.fillScreen(TFT_BLACK);
}
