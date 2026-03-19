#include <TFT_eSPI.h>
#include <images/FreeSansBold55pt7b.h>
#include <modules/app_state.h>
#include <modules/modes/clock_mode.h>
#include <modules/sprites.h>

namespace
{

constexpr int CLOCK_RECT_LEFT_X = 18;
constexpr int CLOCK_RECT_RIGHT_X = 177;
constexpr int CLOCK_RECT_Y = 19;
constexpr int CLOCK_RECT_SIZE = 132;
constexpr int CLOCK_RECT_RADIUS = 14;
constexpr int CLOCK_DOT_X = 158;
constexpr int CLOCK_DOT_TOP_Y = 70;
constexpr int CLOCK_DOT_BOTTOM_Y = 91;
constexpr int CLOCK_DOT_SIZE = 9;
constexpr int CLOCK_DOT_RADIUS = 1;
constexpr int CLOCK_TEXT_BASELINE_Y = 121;
constexpr int CLOCK_HOUR_1_X = 51;
constexpr int CLOCK_HOUR_2_X = 114;
constexpr int CLOCK_MINUTE_1_X = 210;
constexpr int CLOCK_MINUTE_2_X = 273;

String left_digit(int value) { return String((value / 10) % 10); }

String right_digit(int value) { return String(value % 10); }

}  // namespace

void clock_render()
{
    clock_display.setFreeFont(&FreeSansBold55pt7b);
    clock_display.setTextSize(1);
    clock_display.setTextColor(TFT_WHITE);
    clock_display.fillSprite(TFT_BLACK);

    clock_display.fillRoundRect(CLOCK_RECT_LEFT_X, CLOCK_RECT_Y, CLOCK_RECT_SIZE, CLOCK_RECT_SIZE,
                                CLOCK_RECT_RADIUS, TFT_DARKGREY);
    clock_display.fillRoundRect(CLOCK_RECT_RIGHT_X, CLOCK_RECT_Y, CLOCK_RECT_SIZE, CLOCK_RECT_SIZE,
                                CLOCK_RECT_RADIUS, TFT_DARKGREY);
    clock_display.fillRoundRect(CLOCK_DOT_X, CLOCK_DOT_TOP_Y, CLOCK_DOT_SIZE, CLOCK_DOT_SIZE,
                                CLOCK_DOT_RADIUS, TFT_LIGHTGREY);
    clock_display.fillRoundRect(CLOCK_DOT_X, CLOCK_DOT_BOTTOM_Y, CLOCK_DOT_SIZE, CLOCK_DOT_SIZE,
                                CLOCK_DOT_RADIUS, TFT_LIGHTGREY);

    String hour1 = left_digit(globalHours);
    String hour2 = right_digit(globalHours);
    String minute1 = left_digit(globalMinutes);
    String minute2 = right_digit(globalMinutes);

    clock_display.setCursor(CLOCK_HOUR_1_X - (clock_display.textWidth(hour1) / 2),
                            CLOCK_TEXT_BASELINE_Y);
    clock_display.print(hour1);
    clock_display.setCursor(CLOCK_HOUR_2_X - (clock_display.textWidth(hour2) / 2),
                            CLOCK_TEXT_BASELINE_Y);
    clock_display.print(hour2);
    clock_display.setCursor(CLOCK_MINUTE_1_X - (clock_display.textWidth(minute1) / 2),
                            CLOCK_TEXT_BASELINE_Y);
    clock_display.print(minute1);
    clock_display.setCursor(CLOCK_MINUTE_2_X - (clock_display.textWidth(minute2) / 2),
                            CLOCK_TEXT_BASELINE_Y);
    clock_display.print(minute2);

    clock_display.pushSprite(0, 0);
}
