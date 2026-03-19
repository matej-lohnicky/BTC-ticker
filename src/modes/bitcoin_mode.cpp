#include <TFT_eSPI.h>
#include <images/GreenArrow32.h>
#include <images/Japan24.h>
#include <images/RedArrow32.h>
#include <images/UK24.h>
#include <images/US24.h>
#include <images/bitcoin_white_73.h>
#include <images/moon24.h>
#include <modules/app_state.h>
#include <modules/constants.h>
#include <modules/modes/bitcoin_mode.h>
#include <modules/sprites.h>
#include <modules/variables.h>

#include <algorithm>
#include <vector>

namespace
{

constexpr int CHART_STEP = 15;
constexpr int CHART_GRID_HORIZONTAL_LINES = 6;
constexpr int CHART_GRID_VERTICAL_LINES = 10;
constexpr int CHART_GRID_Y_SPACING = 15;
constexpr int CHART_GRID_X_SPACING = 15;
constexpr int CHART_GRID_Y_START = 8;
constexpr int CHART_GRID_X_START = 20;
constexpr int CHART_GRID_LEFT_X = 10;
constexpr int CHART_GRID_RIGHT_X = 165;
constexpr int CHART_GRID_TOP_Y = 0;
constexpr int CHART_GRID_BOTTOM_Y = 91;
constexpr int CHART_FIRST_POINT_X = 155;
constexpr int CHART_FIRST_POINT_Y = 8;
constexpr int CHART_Y_SCALE = 75;
constexpr int CHART_SPRITE_X = 115;
constexpr int CHART_SPRITE_Y = 70;

constexpr int PRICE_CURSOR_X = 0;
constexpr int PRICE_CURSOR_Y = 35;
constexpr int PRICE_SPRITE_X = 118;
constexpr int PRICE_SPRITE_Y = 15;
constexpr int PRICE_MILLION_THRESHOLD = 1000000;

constexpr int PERCENT_TEXT_CENTER_X = 57;
constexpr int PERCENT_TEXT_BASELINE_Y = 28;
constexpr int PERCENT_ARROW_X = 46;
constexpr int PERCENT_ARROW_Y = 42;
constexpr int PERCENT_ARROW_WIDTH = 23;
constexpr int PERCENT_ARROW_HEIGHT = 32;
constexpr int PERCENT_SPRITE_X = 0;
constexpr int PERCENT_SPRITE_Y = 85;

constexpr int BTC_LOGO_X = 0;
constexpr int BTC_LOGO_Y = 0;
constexpr int BTC_LOGO_WIDTH = 73;
constexpr int BTC_LOGO_HEIGHT = 73;
constexpr int BTC_PIVOT_X = 57;
constexpr int BTC_PIVOT_Y = 47;
constexpr int ROTATION_BACK_CIRCLE_X = 37;
constexpr int ROTATION_BACK_CIRCLE_Y = 37;
constexpr int ROTATION_BACK_CIRCLE_RADIUS = 34;
constexpr int ROTATION_SPRITE_X = 20;
constexpr int ROTATION_SPRITE_Y = 10;

constexpr int SESSIONS_MARKET_ICON_X = 4;
constexpr int SESSIONS_US_ICON_Y = 0;
constexpr int SESSIONS_UK_ICON_Y = 24;
constexpr int SESSIONS_JAPAN_ICON_Y = 48;
constexpr int SESSIONS_MOON_ICON_Y = 72;
constexpr int SESSIONS_ICON_WIDTH = 24;
constexpr int SESSIONS_ICON_HEIGHT = 24;
constexpr int SESSIONS_SPRITE_X = 288;
constexpr int SESSIONS_SPRITE_Y = 70;

constexpr int WEEKDAY_MON = 1;
constexpr int WEEKDAY_FRI = 5;
constexpr int UK_OPEN_MINUTE = 540;
constexpr int UK_CLOSE_MINUTE = 1049;
constexpr int US_OPEN_MINUTE = 930;
constexpr int US_CLOSE_MINUTE = 1319;
constexpr int JP_OPEN_MINUTE = 60;
constexpr int JP_CLOSE_MINUTE = 479;

void chart_background()
{
    chart.fillSprite(TFT_BLACK);

    for (int i = 0; i < CHART_GRID_HORIZONTAL_LINES; i++)
    {
        chart.drawLine(CHART_GRID_LEFT_X, i * CHART_GRID_Y_SPACING + CHART_GRID_Y_START,
                       CHART_GRID_RIGHT_X, i * CHART_GRID_Y_SPACING + CHART_GRID_Y_START,
                       TFT_LIGHTGREY);
    }

    for (int i = 0; i < CHART_GRID_VERTICAL_LINES; i++)
    {
        chart.drawLine(i * CHART_GRID_X_SPACING + CHART_GRID_X_START, CHART_GRID_TOP_Y,
                       i * CHART_GRID_X_SPACING + CHART_GRID_X_START, CHART_GRID_BOTTOM_Y,
                       TFT_LIGHTGREY);
    }
}

void display_price_chart()
{
    if (readings.empty())
    {
        return;
    }

    auto minmax = std::minmax_element(readings.begin(), readings.end());
    int curveMin = *minmax.first;
    int curveMax = *minmax.second;

    int curveRange = curveMax - curveMin;
    if (curveRange == 0)
    {
        curveRange = 1;
    }

    int previousX = 0;
    int previousY = 0;

    for (size_t i = 0; i < readings.size(); ++i)
    {
        int priceDeviation = curveMax - readings[i];
        double relativeDeviation = static_cast<double>(priceDeviation) / curveRange;
        int currentX = CHART_FIRST_POINT_X -
                       ((static_cast<int>(readings.size()) - static_cast<int>(i) - 1) * CHART_STEP);
        int currentY = static_cast<int>(CHART_FIRST_POINT_Y + relativeDeviation * CHART_Y_SCALE);

        chart.fillCircle(currentX, currentY, pointRadius, curveColor);
        if (i > 0)
        {
            chart.drawLine(previousX, previousY, currentX, currentY, curveColor);
            chart.drawLine(previousX - 1, previousY, currentX - 1, currentY, curveColor);
            chart.drawLine(previousX + 1, previousY, currentX + 1, currentY, curveColor);
        }

        previousX = currentX;
        previousY = currentY;
    }
}

void display_price()
{
    btc_price.setTextSize(1);
    btc_price.setFreeFont(&FreeMonoBold24pt7b);
    btc_price.setCursor(PRICE_CURSOR_X, PRICE_CURSOR_Y);
    btc_price.fillSprite(TFT_BLACK);
    if (price < PRICE_MILLION_THRESHOLD)
    {
        btc_price.print("$");
    }
    btc_price.print(price);
    btc_price.pushSprite(PRICE_SPRITE_X, PRICE_SPRITE_Y);
}

void display_percent_change()
{
    String output = String(percentChange) + "%";

    btc_percents.fillSprite(TFT_BLACK);
    btc_percents.setTextSize(1);
    btc_percents.setFreeFont(&FreeMonoBold12pt7b);
    btc_percents.setCursor(PERCENT_TEXT_CENTER_X - (btc_percents.textWidth(output) / 2),
                           PERCENT_TEXT_BASELINE_Y);
    btc_percents.print(output);

    if (percentChange > 0)
    {
        btc_percents.pushImage(PERCENT_ARROW_X, PERCENT_ARROW_Y, PERCENT_ARROW_WIDTH,
                               PERCENT_ARROW_HEIGHT, GreenArrow);
    }
    else
    {
        btc_percents.pushImage(PERCENT_ARROW_X, PERCENT_ARROW_Y, PERCENT_ARROW_WIDTH,
                               PERCENT_ARROW_HEIGHT, RedArrow);
    }

    btc_percents.pushSprite(PERCENT_SPRITE_X, PERCENT_SPRITE_Y);
}

void display_btc_logo()
{
    btc_logo.pushImage(BTC_LOGO_X, BTC_LOGO_Y, BTC_LOGO_WIDTH, BTC_LOGO_HEIGHT, bitcoin_icon);
    tft.setPivot(BTC_PIVOT_X, BTC_PIVOT_Y);
}

void sessions_panel()
{
    int minutes = globalHours * 60 + globalMinutes;
    sessions.fillSprite(TFT_BLACK);

    if (globalDay >= WEEKDAY_MON && globalDay <= WEEKDAY_FRI)
    {
        if (minutes >= UK_OPEN_MINUTE && minutes <= UK_CLOSE_MINUTE)
        {
            sessions.pushImage(SESSIONS_MARKET_ICON_X, SESSIONS_UK_ICON_Y, SESSIONS_ICON_WIDTH,
                               SESSIONS_ICON_HEIGHT, UK);
        }
        if (minutes >= US_OPEN_MINUTE && minutes <= US_CLOSE_MINUTE)
        {
            sessions.pushImage(SESSIONS_MARKET_ICON_X, SESSIONS_US_ICON_Y, SESSIONS_ICON_WIDTH,
                               SESSIONS_ICON_HEIGHT, US);
        }
        if (minutes >= JP_OPEN_MINUTE && minutes <= JP_CLOSE_MINUTE)
        {
            sessions.pushImage(SESSIONS_MARKET_ICON_X, SESSIONS_JAPAN_ICON_Y, SESSIONS_ICON_WIDTH,
                               SESSIONS_ICON_HEIGHT, Japan);
        }
        if (minutes > US_CLOSE_MINUTE || minutes < JP_OPEN_MINUTE)
        {
            sessions.pushImage(SESSIONS_MARKET_ICON_X, SESSIONS_MOON_ICON_Y, SESSIONS_ICON_WIDTH,
                               SESSIONS_ICON_HEIGHT, moon);
        }
    }
    else
    {
        sessions.pushImage(SESSIONS_MARKET_ICON_X, SESSIONS_MOON_ICON_Y, SESSIONS_ICON_WIDTH,
                           SESSIONS_ICON_HEIGHT, moon);
    }

    sessions.pushSprite(SESSIONS_SPRITE_X, SESSIONS_SPRITE_Y);
}

}  // namespace

void bitcoin_render()
{
    chart_background();

    if (readings.size() > 1)
    {
        display_price_chart();
    }

    chart.pushSprite(CHART_SPRITE_X, CHART_SPRITE_Y);
    display_price();
    display_percent_change();
    sessions_panel();
}

void bitcoin_logo_rotation()
{
    const unsigned long now = millis();

    if (changed_mode)
    {
        display_btc_logo();
    }

    if (now - lastRotationUpdate >= ROTATION_INTERVAL)
    {
        back_logo.fillCircle(ROTATION_BACK_CIRCLE_X, ROTATION_BACK_CIRCLE_Y,
                             ROTATION_BACK_CIRCLE_RADIUS, TFT_ORANGE);
        btc_logo.pushRotated(&back_logo, angle, TFT_BLACK);
        back_logo.pushSprite(ROTATION_SPRITE_X, ROTATION_SPRITE_Y);

        if (reversal)
        {
            angle++;
            if (angle == angleMax)
            {
                reversal = false;
            }
        }
        else
        {
            angle--;
            if (angle == angleMin)
            {
                reversal = true;
            }
        }

        lastRotationUpdate = now;
    }
}
