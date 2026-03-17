#include <TFT_eSPI.h>
#include <WiFi.h>
#include <modules/constants.h>
#include <modules/sprites.h>
#include <modules/utils.h>
#include <modules/variables.h>
#include <modules/wifi_connect.h>
#include <time.h>

#include <algorithm>
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

const std::vector<String> BOOT_MODES = {"Home", "Standard"};

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
    {
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
    if (wifis.empty())
    {
        show_status_screen("No WiFi networks found");
        delay(MISSING_CREDENTIALS_MESSAGE_DELAY_MS);
        return "";
    }

    unsigned selected = row_choice(wifis);
    if (selected >= wifis.size())
    {
        selected = 0;
    }
    return wifis[selected];
}

void user_input_wifi()
{
    while (true)
    {
        String ssid = choose_wifi();
        if (ssid.isEmpty())
        {
            continue;
        }

        String password = keyboard_input();
        delay(KEYBOARD_CONFIRM_DELAY_MS);

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
    {
        tft.fillScreen(TFT_BLACK);
    }
    else
    {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(STATUS_TEXT_X, STATUS_TEXT_Y);
        tft.print("Failed to connect to WiFi, please try again");
        delay(MISSING_CREDENTIALS_MESSAGE_DELAY_MS);
        user_input_wifi();
    }
}

void connect_wifi()
{
    int selected = row_choice(BOOT_MODES);

    switch (selected)
    {
        case 0:
            fast_connect_wifi();
            break;
        case 1:
            user_input_wifi();
            break;
    }
    tft.fillScreen(TFT_BLACK);
}
