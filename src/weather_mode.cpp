#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <images/clear_sky72.h>
#include <images/cloudy72.h>
#include <images/drizzle72.h>
#include <images/fog72.h>
#include <images/heavy_rain72.h>
#include <images/snowflake72.h>
#include <images/thunderstorm72.h>
#include <modules/app_state.h>
#include <modules/sprites.h>
#include <modules/variables.h>
#include <modules/weather_mode.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace
{

constexpr int WEATHER_HTTP_OK = 200;
constexpr int WEATHER_JSON_CAPACITY = 4096;
constexpr int TIME_SUBSTR_HOUR_START = 11;
constexpr int TIME_SUBSTR_HOUR_END = 13;
constexpr int TIME_SUBSTR_MINUTE_START = 14;
constexpr int TIME_SUBSTR_MINUTE_END = 16;

constexpr int ICON_X = 14;
constexpr int ICON_Y = 7;
constexpr int ICON_WIDTH = 72;
constexpr int ICON_HEIGHT = 72;
constexpr int WEATHER_ICON_SPRITE_X = 0;
constexpr int WEATHER_ICON_SPRITE_Y = 0;

constexpr int TEMP_TEXT_SIZE = 5;
constexpr int TEMP_CENTER_X = 62;
constexpr int TEMP_BASELINE_Y = 25;
constexpr int TEMP_DEGREE_CENTER_X = 70;
constexpr int TEMP_DEGREE_CENTER_Y = 30;
constexpr int TEMP_DEGREE_OUTER_RADIUS = 6;
constexpr int TEMP_SPRITE_X = 100;
constexpr int TEMP_SPRITE_Y = 0;

constexpr int SUN_TEXT_SIZE = 2;
constexpr int SUN_TEXT_BASELINE_TOP_Y = 10;
constexpr int SUN_TEXT_BASELINE_BOTTOM_Y = 60;
constexpr int SUN_TEXT_RIGHT_X = 60;
constexpr int SUN_SPRITE_X = 240;
constexpr int SUN_SPRITE_Y = 0;

constexpr int DAYLIGHT_FRAME_X = 0;
constexpr int DAYLIGHT_FRAME_Y = 5;
constexpr int DAYLIGHT_FRAME_WIDTH = 14;
constexpr int DAYLIGHT_FRAME_HEIGHT = 75;
constexpr int DAYLIGHT_INNER_X = 1;
constexpr int DAYLIGHT_INNER_Y = 6;
constexpr int DAYLIGHT_INNER_WIDTH = 12;
constexpr int DAYLIGHT_INNER_HEIGHT = 73;
constexpr int DAYLIGHT_FILL_X = 2;
constexpr int DAYLIGHT_FILL_Y = 7;
constexpr int DAYLIGHT_FILL_WIDTH = 10;
constexpr int DAYLIGHT_FILL_HEIGHT = 71;
constexpr int DAYLIGHT_SPRITE_X = 305;
constexpr int DAYLIGHT_SPRITE_Y = 0;

constexpr int RAIN_TOP_LINE_Y = 5;
constexpr int RAIN_BOTTOM_LINE_Y = 65;
constexpr int RAIN_LEFT_X = 16;
constexpr int RAIN_TOP_RIGHT_X = 303;
constexpr int RAIN_BOTTOM_RIGHT_X = 301;
constexpr int RAIN_HOUR_LABEL_COUNT = 9;
constexpr int RAIN_HOUR_STEP = 3;
constexpr int RAIN_HOUR_X_STEP = 36;
constexpr int RAIN_HOUR_TEXT_OFFSET_X = 5;
constexpr int RAIN_HOUR_TEXT_Y = 70;
constexpr int RAIN_BAR_MAX_COUNT = 24;
constexpr int RAIN_BAR_X_STEP = 12;
constexpr int RAIN_BAR_WIDTH = 10;
constexpr int RAIN_BAR_MAX_HEIGHT = 59;
constexpr int RAIN_SPRITE_X = 0;
constexpr int RAIN_SPRITE_Y = 85;

constexpr const char API_WEATHER[] =
    "https://api.open-meteo.com/v1/"
    "forecast?latitude=50.2092&longitude=15.8328&current=temperature_2m,"
    "weather_code&hourly=precipitation_probability&daily=sunrise,sunset&"
    "timezone=auto&forecast_days=1";

void update_weather()
{
    HTTPClient http;
    http.begin(API_WEATHER);
    int httpCode = http.GET();

    if (httpCode == WEATHER_HTTP_OK)
    {
        String payload = http.getString();
        DynamicJsonDocument doc(WEATHER_JSON_CAPACITY);

        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
            Serial.print("Failed to parse JSON. Error: ");
            Serial.println(error.c_str());
            http.end();
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

        sunriseHours = sunrise.substring(TIME_SUBSTR_HOUR_START, TIME_SUBSTR_HOUR_END).toInt();
        sunriseMinutes =
            sunrise.substring(TIME_SUBSTR_MINUTE_START, TIME_SUBSTR_MINUTE_END).toInt();
        sunsetHours = sunset.substring(TIME_SUBSTR_HOUR_START, TIME_SUBSTR_HOUR_END).toInt();
        sunsetMinutes = sunset.substring(TIME_SUBSTR_MINUTE_START, TIME_SUBSTR_MINUTE_END).toInt();

        precipitationProbability.clear();
        for (size_t i = 0; i < precipitationArray.size(); i++)
        {
            precipitationProbability.push_back(precipitationArray[i].as<int>());
        }
    }

    http.end();
}

void display_weather_icon()
{
    weatherIcon.fillSprite(TFT_BLACK);
    const uint16_t* iconData = cloudy72;

    switch (weatherCode)
    {
        case 0:
            iconData = clear_sky72;
            break;
        case 1 ... 3:
            iconData = cloudy72;
            break;
        case 45 ... 48:
            iconData = fog72;
            break;
        case 51 ... 57:
        case 80 ... 82:
            iconData = drizzle72;
            break;
        case 61 ... 67:
            iconData = heavy_rain72;
            break;
        case 71 ... 77:
        case 85 ... 86:
            iconData = snowflake72;
            break;
        case 95 ... 99:
            iconData = thunderstorm72;
            break;
    }

    weatherIcon.pushImage(ICON_X, ICON_Y, ICON_WIDTH, ICON_HEIGHT, iconData);
    weatherIcon.pushSprite(WEATHER_ICON_SPRITE_X, WEATHER_ICON_SPRITE_Y);
}

void display_temperature()
{
    temperature.fillSprite(TFT_BLACK);
    temperature.setFreeFont();
    temperature.setTextSize(TEMP_TEXT_SIZE);

    String outputCurrentTemperature = String(currentTemperature, 1);
    temperature.setCursor(TEMP_CENTER_X - (temperature.textWidth(outputCurrentTemperature) / 2),
                          TEMP_BASELINE_Y);
    temperature.print(outputCurrentTemperature);
    temperature.drawCircle(
        TEMP_DEGREE_CENTER_X + (temperature.textWidth(outputCurrentTemperature) / 2),
        TEMP_DEGREE_CENTER_Y, TEMP_DEGREE_OUTER_RADIUS, TFT_WHITE);
    temperature.drawCircle(
        TEMP_DEGREE_CENTER_X + (temperature.textWidth(outputCurrentTemperature) / 2),
        TEMP_DEGREE_CENTER_Y, TEMP_DEGREE_OUTER_RADIUS - 1, TFT_WHITE);
    temperature.drawCircle(
        TEMP_DEGREE_CENTER_X + (temperature.textWidth(outputCurrentTemperature) / 2),
        TEMP_DEGREE_CENTER_Y, TEMP_DEGREE_OUTER_RADIUS - 2, TFT_WHITE);

    temperature.pushSprite(TEMP_SPRITE_X, TEMP_SPRITE_Y);
}

void display_sun_time()
{
    temperatureExtremes.fillSprite(TFT_BLACK);
    temperatureExtremes.setTextSize(SUN_TEXT_SIZE);

    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d:%02d", sunriseHours, sunriseMinutes);
    temperatureExtremes.setCursor(SUN_TEXT_RIGHT_X - temperatureExtremes.textWidth(buffer),
                                  SUN_TEXT_BASELINE_TOP_Y);
    temperatureExtremes.print(buffer);

    snprintf(buffer, sizeof(buffer), "%d:%02d", sunsetHours, sunsetMinutes);
    temperatureExtremes.setCursor(SUN_TEXT_RIGHT_X - temperatureExtremes.textWidth(buffer),
                                  SUN_TEXT_BASELINE_BOTTOM_Y);
    temperatureExtremes.print(buffer);

    temperatureExtremes.pushSprite(SUN_SPRITE_X, SUN_SPRITE_Y);
}

void display_sun_time_range()
{
    temperatureRange.fillSprite(TFT_BLACK);

    temperatureRange.drawRect(DAYLIGHT_FRAME_X, DAYLIGHT_FRAME_Y, DAYLIGHT_FRAME_WIDTH,
                              DAYLIGHT_FRAME_HEIGHT, TFT_WHITE);
    temperatureRange.drawRect(DAYLIGHT_INNER_X, DAYLIGHT_INNER_Y, DAYLIGHT_INNER_WIDTH,
                              DAYLIGHT_INNER_HEIGHT, TFT_WHITE);
    temperatureRange.fillRect(DAYLIGHT_FILL_X, DAYLIGHT_FILL_Y, DAYLIGHT_FILL_WIDTH,
                              DAYLIGHT_FILL_HEIGHT, TFT_ORANGE);

    int totalSunriseMinutes = (sunriseHours * 60) + sunriseMinutes;
    int totalSunsetMinutes = (sunsetHours * 60) + sunsetMinutes;
    int totalCurrentMinutes = (globalHours * 60) + globalMinutes;
    int dayMinutes = totalSunsetMinutes - totalSunriseMinutes;

    if (dayMinutes <= 0)
    {
        temperatureRange.fillRect(DAYLIGHT_FILL_X, DAYLIGHT_FILL_Y, DAYLIGHT_FILL_WIDTH,
                                  DAYLIGHT_FILL_HEIGHT, TFT_BLACK);
        temperatureRange.pushSprite(DAYLIGHT_SPRITE_X, DAYLIGHT_SPRITE_Y);
        return;
    }

    double relativeDeviation = static_cast<double>(totalCurrentMinutes - totalSunriseMinutes) /
                               static_cast<double>(dayMinutes);

    if (relativeDeviation <= 1 && relativeDeviation >= 0)
    {
        temperatureRange.fillRect(DAYLIGHT_FILL_X, DAYLIGHT_FILL_Y, DAYLIGHT_FILL_WIDTH,
                                  static_cast<int>(DAYLIGHT_FILL_HEIGHT * relativeDeviation),
                                  TFT_BLACK);
    }
    else
    {
        temperatureRange.fillRect(DAYLIGHT_FILL_X, DAYLIGHT_FILL_Y, DAYLIGHT_FILL_WIDTH,
                                  DAYLIGHT_FILL_HEIGHT, TFT_BLACK);
    }

    temperatureRange.pushSprite(DAYLIGHT_SPRITE_X, DAYLIGHT_SPRITE_Y);
}

void display_rain_chart()
{
    rainChart.fillSprite(TFT_BLACK);
    rainChart.setTextSize(1);

    rainChart.drawLine(RAIN_LEFT_X, RAIN_TOP_LINE_Y, RAIN_TOP_RIGHT_X, RAIN_TOP_LINE_Y, TFT_WHITE);
    rainChart.drawLine(RAIN_LEFT_X, RAIN_BOTTOM_LINE_Y, RAIN_BOTTOM_RIGHT_X, RAIN_BOTTOM_LINE_Y,
                       TFT_WHITE);

    String outputHourNumber;
    for (int i = 0; i < RAIN_HOUR_LABEL_COUNT; ++i)
    {
        outputHourNumber = String(i * RAIN_HOUR_STEP);
        rainChart.setCursor(RAIN_LEFT_X + RAIN_HOUR_X_STEP * i + RAIN_HOUR_TEXT_OFFSET_X -
                                (rainChart.textWidth(outputHourNumber) / 2),
                            RAIN_HOUR_TEXT_Y);
        rainChart.print(i * RAIN_HOUR_STEP);
    }

    int bars = std::min(static_cast<int>(precipitationProbability.size()), RAIN_BAR_MAX_COUNT);
    for (int j = 0; j < bars; ++j)
    {
        int y = static_cast<int>(round(precipitationProbability[j] / 100.0 * RAIN_BAR_MAX_HEIGHT));
        rainChart.fillRect(RAIN_LEFT_X + RAIN_BAR_X_STEP * j, RAIN_BOTTOM_LINE_Y - y,
                           RAIN_BAR_WIDTH, y, TFT_BLUE);
    }

    rainChart.pushSprite(RAIN_SPRITE_X, RAIN_SPRITE_Y);
}

}  // namespace

void weather_render()
{
    if (!offline_mode)
    {
        update_weather();
    }

    display_weather_icon();
    display_temperature();
    display_sun_time();
    display_sun_time_range();
    display_rain_chart();
}
