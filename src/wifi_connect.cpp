#include <TFT_eSPI.h>
#include <WiFi.h>
#include <time.h>
#include <vector>

#include <modules/wifi_connect.h>
#include <modules/sprites.h>
#include <modules/utils.h>
#include <modules/constants.h>
#include <modules/variables.h>
#include <credentials/wifi_credentials.h>


std::vector<String> get_available_wifis()
{
    std::vector<String> result;
    
    for (int i = 0; i < 3; i++) {  // searching for networks, 3 times to ensure seeing all of them
        tft.print(".");
        int numNetworks = WiFi.scanNetworks();
        
        for (int i = 0; i < numNetworks; i++) {
            String wifiName = WiFi.SSID(i);
            if (std::find(result.begin(), result.end(), wifiName) == result.end()) {
                result.push_back(wifiName);
            }
        }
    }

    return result;
}


String choose_wifi()
{
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 20);
    tft.print("Loading networks");
    
    const std::vector<String> wifis = get_available_wifis();
    unsigned selected = row_choice(wifis);
    return wifis[selected].c_str();
}


void user_input_wifi()
{
    while (true) {
        String ssid = choose_wifi();
        String password = keyboard_input();
        delay(500);  // KEEP SOME DELAY, for the buttons to work properly

        tft.fillScreen(TFT_BLACK);
        tft.setCursor(20, 20);

        WiFi.begin(ssid, password);
        tft.print("Connecting to WiFi");

        for (int i = 0; WiFi.status() != WL_CONNECTED && i < 20; ++i) {
            delay(1000);
            tft.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            tft.fillScreen(TFT_BLACK);
            return;
        }
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(20, 20);
        tft.print("Failed to connect to WiFi please try again");
        delay(4);
    }
}


void fast_connect_wifi() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 20);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    tft.print("Connecting to WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(1000);
        tft.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {  // WiFi connected successfully 
        tft.fillScreen(TFT_BLACK);
    } else {  // Wi-Fi connection failed
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(20, 20);
        tft.print("Failed to connect to WiFi, please try again");
        while(true){}  // prevets further code execution without the internet
    }
}


void connect_wifi() {
    const std::vector<String> BOOT_MODES = {"Home", "Standard", 
                                            "Showcase", "Offline"};
    int selected = row_choice(BOOT_MODES);

    switch (selected) {
        case 0:  // home, credentials from firmware
            fast_connect_wifi();
            chartFasterMode = false;
            mode0_update_interval = MODE0_UPDATE_INTERVAL_NORMAL;
            break;
        case 1:  // outside from home, credentials from user
            user_input_wifi();
            chartFasterMode = false;
            mode0_update_interval = MODE0_UPDATE_INTERVAL_NORMAL;
            break;
        case 2:  // showcase, faster
            user_input_wifi();
            chartFasterMode = true;
            mode0_update_interval = MODE0_UPDATE_INTERVAL_FAST;
            break;
        case 3:  // offline mode, random price generator
            assert(false);  // TBD
            /*
            offline_mode = true;
            readings = {68671};
            price = 68671;
            percentChange = 1.52;
            globalHours = 9;
            globalMinutes = 32;
            globalDay = 1;
            sunriseHours = 4;
            sunriseMinutes = 58;
            sunsetHours = 20;
            sunsetMinutes = 44;
            mode0_update_interval = MODE0_UPDATE_INTERVAL_FAST;
            precipitationProbability = {0,0,0,0,0,10,15,20,40,60,60,30,20,10,5,15,20,10,5,0,0,0,0,0};
            weatherCode = 1;
            currentTemperature = 22.5;
            break;
            */
    }
}
