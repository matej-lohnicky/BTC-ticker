#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include <cmath>
#include <random>
#include <time.h>
#include <algorithm>

#include <modules/wifi_connect.h>
#include <modules/sprites.h>
#include <modules/constants.h>
#include <modules/variables.h>

// bitcoin logo
#include <images/bitcoin_white_73.h>
// session panel icons
#include <images/US24.h> 
#include <images/UK24.h>
#include <images/Japan24.h>
#include <images/moon24.h>
// arrows for the relative price change
#include <images/GreenArrow32.h>
#include <images/RedArrow32.h>
// clock font
#include <images/FreeSansBold55pt7b.h>
// weather icons
#include <images/clear_sky72.h>
#include <images/cloudy72.h>
#include <images/fog72.h>
#include <images/drizzle72.h>
#include <images/heavy_rain72.h>
#include <images/snowflake72.h>
#include <images/thunderstorm72.h>

#define LCD_BACKLIGHT (38)  // display brightness

// APIs
const char *API_BTC_PRICE = "https://api.binance.com/api/v3/ticker/24hr?symbol=BTCUSDT";
const char *API_WEATHER = "https://api.open-meteo.com/v1/forecast?latitude=50.2092&longitude=15.8328&current=temperature_2m,weather_code&hourly=precipitation_probability&daily=sunrise,sunset&timezone=auto&forecast_days=1";

// modes
int mode = 0; //screen mode
bool changed_mode = true;

// BTC logo rotation settings
int angle = 0;
int angleMax = 30;
int angleMin = -5;
bool reversal = true;

// Brightness settings
byte currentBrightness = 120;
const byte DAY_BRIGHTNESS = 250;
const byte EVENING_BRIGHTNESS = 50;
const byte NIGHT_BRIGHTNESS = 5;

// price and chart variables/settings
int pointRadius = 3;  // points in the chart
int curveColor = TFT_BLUE;
int price, chartTime;
double percentChange;
int chartTimeChange = 0;  // last update time
std::vector<int> readings;

//time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;  // UTC+1
const int   daylightOffset_sec = 3600;  // +1 hour for summer time
int globalMinutes, globalHours, globalDay;
int offline_minute_update;

// weather
int weatherCode, sunriseHours, sunriseMinutes, sunsetHours, sunsetMinutes;
float currentTemperature;
std::vector<int> precipitationProbability;

// offline random price generator
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> distribution(-300, 300);

// functions
void update_price();
void display_price();
void display_btc_logo();
void logo_rotation();
void sessions_panel();
void update_time();
void update_price_chart();  // change (delete minutes) int& globalMinutes,
void display_price_chart();
void chart_background();
void display_percent_change();
void update_weather();
void display_weather_icon();
void display_temperature();
void display_sun_time();
void display_sun_time_range();
void display_rain_chart();
void adjust_brightness();
void mode_bitcoin();
void mode_clock();
void mode_weather();
void update_mode_variables();
void sync_chart();


void setup() {
    Serial.begin(115200);
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    analogWrite(LCD_BACKLIGHT, currentBrightness);
    
    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
    
    back_logo.createSprite(74, 74);
    btc_logo.createSprite(73, 73);
    btc_price.createSprite(205, 45);
    chart.createSprite(173, 100);
    sessions.createSprite(32, 100);
    btc_percents.createSprite(115, 86);
    clock_display.createSprite(320, 170);
    weatherIcon.createSprite(100, 85);
    temperature.createSprite(140, 85);
    temperatureExtremes.createSprite(66, 85);
    temperatureRange.createSprite(14, 85);
    rainChart.createSprite(320, 85);

    btc_logo.setSwapBytes(true);
    sessions.setSwapBytes(true);
    btc_percents.setSwapBytes(true);
    weatherIcon.setSwapBytes(true);
    temperatureExtremes.setSwapBytes(true);
    
    connect_wifi();
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    update_price();
    update_time();
    adjust_brightness();
    sync_chart();
}


void loop() {
    currentMillis = millis();

    if (digitalRead(BUTTON_UP) == LOW) {
        mode = (mode + 1) % 3;
        changed_mode = true;
        tft.fillScreen(TFT_BLACK);
        delay(250);
    }
    if (digitalRead(BUTTON_DOWN) == LOW) {
        mode = (mode - 1 + 3) % 3;
        changed_mode = true;
        tft.fillScreen(TFT_BLACK);
        delay(250);
    }

    if (currentMillis - lastPriceUpdate >= PRICE_UPDATE_INTERVAl) {
        update_price();
    }

    if (currentMillis - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
        update_time();
        adjust_brightness();
        sync_chart();
    }

    switch(mode) {
        case 0:
            logo_rotation();
            if (currentMillis - lastModeUpdate >= MODE0_UPDATE_INTERVAL_NORMAL || changed_mode) {
                mode_bitcoin();
                update_mode_variables();
            }
            break;
        case 1:
            if (currentMillis - lastModeUpdate >= MODE1_UPDATE_INTERVAL || changed_mode) {
                mode_clock();
                update_mode_variables();
            }
            break;
        case 2:
            if (currentMillis - lastModeUpdate >= MODE2_UPDATE_INTERVAL || changed_mode) {
                mode_weather();
                update_mode_variables();
            }
            break;
    }
}

void mode_bitcoin() {
    chart_background();

    if (readings.size() > 1) {
        display_price_chart();
    }

    chart.pushSprite(115, 70);
    display_price();
    display_percent_change();
    sessions_panel();
}

void sync_chart()
{
    if (chartTimeChange != chartTime) { 
        update_price_chart();
        chartTimeChange = chartTime;
    }
}

void mode_clock() {
    clock_display.setFreeFont(&FreeSansBold55pt7b);
    clock_display.setTextSize(1);
    clock_display.setTextColor(TFT_WHITE);
    clock_display.fillSprite(TFT_BLACK);
    
    // clock design
    clock_display.fillRoundRect(15 + 8 - 5, 20 - 1, 132, 132, 14, TFT_DARKGREY);
    clock_display.fillRoundRect(180 - 8 + 5, 20 - 1, 132, 132, 14, TFT_DARKGREY);
    clock_display.fillRoundRect(158, 70, 9, 9, 1, TFT_LIGHTGREY);
    clock_display.fillRoundRect(158, 91, 9, 9, 1, TFT_LIGHTGREY);
    
    String Hour1, Hour2, Minute1, Minute2;
    if (globalHours < 10) {
        Hour1 = String(0);
        Hour2 = String(globalHours);
    } else {
        Hour1 = String(globalHours)[0];
        Hour2 = String(globalHours)[1];
    }
    
    if (globalMinutes < 10) {
        Minute1 = String(0);
        Minute2 = String(globalMinutes);
    } else {
        Minute1 = String(globalMinutes)[0];
        Minute2 = String(globalMinutes)[1];
    }
    
    clock_display.setCursor(51 - (clock_display.textWidth(Hour1) / 2), 30 + 90 + 1);
    clock_display.print(Hour1);
    clock_display.setCursor(117 - 3 - (clock_display.textWidth(Hour2) / 2), 30 + 90 + 1);
    clock_display.print(Hour2);
    clock_display.setCursor(210 - (clock_display.textWidth(Minute1) / 2), 30 + 90 + 1);
    clock_display.print(Minute1);
    clock_display.setCursor(276 - 3 - (clock_display.textWidth(Minute2) / 2), 30 + 90 + 1);
    clock_display.print(Minute2);

    clock_display.pushSprite(0, 0);
}

void mode_weather() {
    if (!offline_mode) {
        update_weather();
    }
    display_weather_icon();
    display_temperature();
    display_sun_time();
    display_sun_time_range();
    display_rain_chart();
}

void update_mode_variables() {
    changed_mode = false;
    lastModeUpdate = currentMillis;
}


void update_price() {
    lastPriceUpdate = currentMillis;

    if (offline_mode) {
        if (!changed_mode) return;
    
        int last_reading = readings.back();
        int new_reading = last_reading + distribution(gen);
        readings.push_back(new_reading);
        price = new_reading;

        if (readings.size() > 10) {
            readings.erase(readings.begin());
        }
        percentChange += (static_cast<double>(new_reading) / last_reading - 1) * 100;
        if (readings.size()>1) {
            display_price_chart();
        }
        return;
    }

    btc_price.setFreeFont(&FreeMonoBold24pt7b);

    HTTPClient http;
    http.begin(API_BTC_PRICE);
    int httpCode = http.GET();

    if (httpCode > 0) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print("Failed to parse JSON. Error: ");
            Serial.println(error.c_str());
        } else {
            double lastPriceDouble = doc["lastPrice"].as<double>();
            price = static_cast<int>(lastPriceDouble);
            percentChange = doc["priceChangePercent"].as<double>();
        }
    } else {
        Serial.print("HTTP GET failed. Code: ");
        Serial.println(httpCode);
    }

    http.end();
}

void display_price() {
    btc_price.setTextSize(1);
    btc_price.setFreeFont(&FreeMonoBold24pt7b);
    btc_price.setCursor(0, 35);
    btc_price.fillSprite(TFT_BLACK);  // clears the old price
    if (price < 1000000) {  // handles price width when over $1M
        btc_price.print("$");
    }
    btc_price.print(price);
    btc_price.pushSprite(118, 15);
}

void display_percent_change() {
    String FinalOutput;
    btc_percents.fillSprite(TFT_BLACK);
    btc_percents.setTextSize(1);
    btc_percents.setFreeFont(&FreeMonoBold12pt7b);

    FinalOutput = String(percentChange) + "%";
    btc_percents.setCursor(57 - (btc_percents.textWidth(FinalOutput) / 2), 28);
    btc_percents.print(FinalOutput);

    if (percentChange>0) {
        btc_percents.pushImage(46, 42, 23, 32, GreenArrow);
    } else {
        btc_percents.pushImage(46, 42, 23, 32, RedArrow);
    }

    btc_percents.pushSprite(0, 85);
}

void display_btc_logo(){
    btc_logo.pushImage(0, 0, 73, 73, bitcoin_icon);
    tft.setPivot(57, 47);
}

void logo_rotation() {
    unsigned long currentMillis = millis();

    if (changed_mode) {
        display_btc_logo();
    }

    if (currentMillis - lastRotationUpdate >= ROTATION_INTERVAL) {
        back_logo.fillCircle(37, 37, 34, TFT_ORANGE);
        btc_logo.pushRotated(&back_logo, angle, TFT_BLACK);
        back_logo.pushSprite(20, 10);

        if (reversal) {
            angle++;
            if (angle == angleMax) {
                delay(50);
                reversal = false;  // Change direction when reaching andleMAX
            }
        } else {
            angle--;
            if (angle == angleMin) {
                delay(50);
                reversal = true;  // Change direction when reaching angleMin
            }
        }

    lastRotationUpdate = currentMillis;
    }
}

void sessions_panel() {
    int Minutes = globalHours * 60 + globalMinutes;
    sessions.fillSprite(TFT_BLACK);

    if (globalDay>0 and globalDay<6){
        if (Minutes>539 and Minutes<1050) {
            sessions.pushImage(4,24,24,24,UK);
        }
        if (Minutes>929 and Minutes<1320) {
            sessions.pushImage(4,0,24,24,US);
        }
        if (Minutes>59 and Minutes<480) {
            sessions.pushImage(4,48,24,24,Japan);
        }
        if (Minutes>1319 or Minutes<60){
            sessions.pushImage(4,72,24,24,moon);
        }
    } else {  // weekends
        sessions.pushImage(4,72,24,24,moon);
    }
    sessions.pushSprite(288,70);
}

void update_time() {
    lastTimeUpdate = currentMillis;
    
    if (offline_mode == true) { 
        if (currentMillis <= offline_minute_update + 60000) return;
        globalMinutes += 1;
        if (globalMinutes > 59) {
            globalMinutes = 0;
            globalHours += 1;
        }
        offline_minute_update = currentMillis;
        return;
    }

    struct tm timeinfo;

    while (!getLocalTime(&timeinfo)) {
        Serial.println("Waiting for local time sync...");
        delay(1000);
    }
    Serial.println("Time from local synchronized");
    
    globalMinutes = timeinfo.tm_min;
    globalHours = timeinfo.tm_hour;
    globalDay = timeinfo.tm_wday;

    if (chartFasterMode) {
        chartTime = globalMinutes;
    } else {
        chartTime = globalHours;
    }
}

void chart_background() {
    int x = 6;
    int y = 10;
    chart.fillSprite(TFT_BLACK);
    for (int i = 0; i < x; i++) {
        chart.drawLine(10, i * 15 + 8, 165, i * 15 + 8, TFT_LIGHTGREY);  // horizontal (x) chart
    }
    for (int i = 0; i < y; i++) {
        chart.drawLine(i * 15 + 20, 0, i * 15 + 20, 91, TFT_LIGHTGREY);  // vertical (y) chart
    }
}

void update_price_chart() {
    if ((readings.size()) < 10) {
        readings.push_back(price);
    } else {
        for (int i = 0; i < 9; i++) {
            readings[i] = readings[i+1];
        }
        readings[9] = price;
    }
}

void display_price_chart() {
    auto minmax = std::minmax_element(readings.begin(), readings.end());
    int curveMin = *minmax.first;
    int curveMax = *minmax.second;

    int curveRange = curveMax-curveMin;
    int PriceDeviation;
    double relativeDeviation;
    int previousX, previousY, currentX, currentY;

    for (int i = 0; i < (readings.size()); ++i) {
        PriceDeviation = curveMax-readings[i];
        relativeDeviation = static_cast<double>(PriceDeviation)/curveRange;
        currentX = 155 - ((readings.size() - i - 1) * 15);
        currentY = 8 + relativeDeviation * 75;

        chart.fillCircle(currentX, currentY, pointRadius, curveColor);
        if (i > 0) {
            chart.drawLine(previousX, previousY, currentX, currentY, curveColor);
            chart.drawLine(previousX-1, previousY, currentX-1, currentY, curveColor);
            chart.drawLine(previousX+1, previousY, currentX+1, currentY, curveColor);
        }
        previousX = currentX;
        previousY = currentY;
    }
}


void update_weather() {
    HTTPClient http;
    http.begin(API_WEATHER);
    int httpCode = http.GET();

    if (httpCode > 0) {
        String payload = http.getString();
        DynamicJsonDocument doc(4096);

        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print("Failed to parse JSON. Error: ");
            Serial.println(error.c_str());
            return;
        }
        
        JsonObject current = doc["current"];
        JsonObject daily = doc["daily"];
        JsonObject hourly = doc["hourly"];
        JsonArray precipitationArray = hourly["precipitation_probability"];
        weatherCode = static_cast<int>(current["weather_code"].as<double>());
        currentTemperature = static_cast<float>(current["temperature_2m"].as<double>());
        String sunrise = daily["sunrise"][0].as<String>();
        String sunset = daily["sunset"][0].as<String>();

        sunriseHours = (sunrise.substring(11, 13)).toInt();
        sunriseMinutes = (sunrise.substring(14, 16)).toInt();
        sunsetHours = (sunset.substring(11, 13)).toInt();
        sunsetMinutes = (sunset.substring(14, 16)).toInt();

        precipitationProbability.clear();
        for (int i = 0; i < precipitationArray.size(); i++) {
            precipitationProbability.push_back(precipitationArray[i].as<int>());
        }
    }

    http.end();
    }

void display_weather_icon() {
    weatherIcon.fillSprite(TFT_BLACK);
    const uint16_t* iconData;

    switch (weatherCode) {
    case 0:
        iconData = clear_sky72;
        break;
    case 1 ... 3:
        iconData = cloudy72;
        break;
    case 45 ... 48:
        iconData = fog72;
        break;
    case 51 ... 57: case 80 ... 82:
        iconData = drizzle72;
        break;
    case 61 ... 67:
        iconData = heavy_rain72;
        break;
    case 71 ... 77: case 85 ... 86:
        iconData = snowflake72;
        break;
    case 95 ... 99:
        iconData = thunderstorm72;
        break;
    }

    weatherIcon.pushImage(14, 7, 72, 72, iconData);
    weatherIcon.pushSprite(0, 0);
}

void display_temperature() {
    temperature.fillSprite(TFT_BLACK);
    temperature.setFreeFont();
    temperature.setTextSize(5);

    String outputCurrentTemperature = String(currentTemperature, 1);
    temperature.setCursor(70 - 8 - (temperature.textWidth(outputCurrentTemperature) / 2), 25);
    temperature.print(outputCurrentTemperature);
    temperature.drawCircle(70 + (temperature.textWidth(outputCurrentTemperature) / 2), 30, 6, TFT_WHITE);
    temperature.drawCircle(70 + (temperature.textWidth(outputCurrentTemperature) / 2), 30, 5, TFT_WHITE);
    temperature.drawCircle(70 + (temperature.textWidth(outputCurrentTemperature) / 2), 30, 4, TFT_WHITE);

    temperature.pushSprite(100, 0);
}

void display_sun_time() {
    temperatureExtremes.fillSprite(TFT_BLACK);
    temperatureExtremes.setTextSize(2);
    
    // Format sunrise time
    char buffer[10];
    sprintf(buffer, "%d:%02d", sunriseHours, sunriseMinutes);
    temperatureExtremes.setCursor(60 - (temperatureExtremes.textWidth(buffer)), 10);
    temperatureExtremes.print(buffer);
    // Format sunset time
    sprintf(buffer, "%d:%02d", sunsetHours, sunsetMinutes);
    temperatureExtremes.setCursor(60 - (temperatureExtremes.textWidth(buffer)), 60);
    temperatureExtremes.print(buffer);

    temperatureExtremes.pushSprite(240, 0);
}

void display_sun_time_range() {
    temperatureRange.fillSprite(TFT_BLACK);

    // frame
    temperatureRange.drawRect(0, 5, 14, 75, TFT_WHITE);
    temperatureRange.drawRect(1, 6, 12, 73, TFT_WHITE);
    temperatureRange.fillRect(2, 7, 10, 71, TFT_ORANGE);

    // shrinking the remaining daylight
    int totalSunriseMinutes = (sunriseHours * 60) + sunriseMinutes;
    int totalSunsetMinutes = (sunsetHours * 60) + sunsetMinutes;
    int totalCurrentMinutes = (globalHours * 60) + globalMinutes;
    double relativeDeviation = static_cast<double>(totalCurrentMinutes - totalSunriseMinutes) / (totalSunsetMinutes - totalSunriseMinutes);
    if (relativeDeviation<=1 && relativeDeviation >= 0) {
        temperatureRange.fillRect(2, 7, 10, (71 * relativeDeviation), TFT_BLACK);
    } else {
        temperatureRange.fillRect(2, 7, 10, 71, TFT_BLACK);
    }

    temperatureRange.pushSprite(305, 0);
}

void display_rain_chart() {
    rainChart.fillSprite(TFT_BLACK);
    rainChart.setTextSize(1);

    // frame
    rainChart.drawLine(16, 5, 303, 5, TFT_WHITE);
    rainChart.drawLine(16, 5 + 60, 301, 5 + 60, TFT_WHITE);
    
    // hour line below the chart
    String outputHourNumber;
    for (int i = 0; i < 9; ++i) {
        outputHourNumber=String(i*3);
        rainChart.setCursor(16 + 36*i + 5 - (rainChart.textWidth(outputHourNumber) / 2), 70);
        rainChart.print(i*3);
    }
    
    // chart columns
    int y;
    for (int j = 0; j < 24; ++j) {
        y = static_cast<int>(round(precipitationProbability[j] / 100.0 * 59));
        rainChart.fillRect(16 + 12*j, 5 + 60 - y, 10, y, TFT_BLUE);
    }

    rainChart.pushSprite(0, 85);
}

void adjust_brightness() {
    byte newBrightness;

    switch (globalHours){
        case 6 ... 17:
        newBrightness = DAY_BRIGHTNESS;
        break;
        case 18 ... 21:
        if (globalHours < sunsetHours) {
            newBrightness = DAY_BRIGHTNESS;
        } else {
            newBrightness = EVENING_BRIGHTNESS;
        }
        break;
        case 22 ... 23: case 0 ... 5:
        newBrightness = NIGHT_BRIGHTNESS;
        break;
    }

    if (newBrightness > currentBrightness) {
        while (!(newBrightness == currentBrightness)) {
            currentBrightness++;
            analogWrite(LCD_BACKLIGHT, currentBrightness);
            delay(50);
        }
    } else if (newBrightness < currentBrightness) {
        while (!(newBrightness == currentBrightness)) {
            --currentBrightness;
            analogWrite(LCD_BACKLIGHT, currentBrightness);
            delay(50);
        }
    }
}
