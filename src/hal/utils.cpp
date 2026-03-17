#include <TFT_eSPI.h>
#include <modules/constants.h>
#include <modules/sprites.h>
#include <modules/variables.h>

#include <cstddef>
#include <vector>

const std::vector<String> KEYS = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "a", "b", "c",
                                  "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p",
                                  "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", ".", "+"};

namespace
{

constexpr int SCREEN_WIDTH = 320;
constexpr int SCREEN_LAST_X = SCREEN_WIDTH - 1;

constexpr int MENU_FRAME_TOP_OFFSET = 3;
constexpr int MENU_FRAME_HEIGHT = row_height + 1;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 250UL;

constexpr int GRID_COLS = 10;
constexpr int GRID_ROWS = 4;
constexpr int GRID_CELL_SIZE = 32;
constexpr int GRID_TOP_Y = 42;
constexpr int GRID_BOTTOM_Y = GRID_TOP_Y + GRID_CELL_SIZE * GRID_ROWS - 1;
constexpr int GRID_TEXT_OFFSET_X = 12;
constexpr int GRID_TEXT_OFFSET_Y = 9;

constexpr int INPUT_TEXT_X = 5;
constexpr int INPUT_TEXT_Y = 5;
constexpr int INPUT_CLEAR_HEIGHT = 40;

constexpr int KEYBOARD_START_COL = 0;
constexpr int KEYBOARD_START_ROW = 1;
constexpr int BACKSPACE_KEY_INDEX = 38;
constexpr int SHIFT_KEY_INDEX = 39;

constexpr unsigned long KEYBOARD_NAV_DELAY_MS = 100UL;
constexpr unsigned long ROW_CHOICE_TIMEOUT_MS = 120000UL;
constexpr unsigned long KEYBOARD_INPUT_TIMEOUT_MS = 180000UL;

inline int grid_cell_x(int col) { return col * GRID_CELL_SIZE; }
inline int grid_cell_y(int row) { return GRID_TOP_Y + row * GRID_CELL_SIZE; }

inline void draw_keyboard_selector(int col, int row, Color color)
{
    tft.drawRect(grid_cell_x(col), grid_cell_y(row), GRID_CELL_SIZE + 1, GRID_CELL_SIZE + 1, color);
}

}  // namespace

void display_rows(const std::vector<String>& rows)
{
    tft.fillScreen(TFT_BLACK);
    int y = row_name_y_padding;

    for (const auto& reading : rows)
    {
        tft.setCursor(row_name_x_padding, y);
        tft.print(reading);
        y += row_height;
    }
}

unsigned row_choice(const std::vector<String>& rows)
{
    if (rows.empty())
    {
        return 0;
    }

    display_rows(rows);
    int y = row_name_y_padding;
    int row_index = 0;
    const unsigned long startMillis = millis();

    auto draw_row_frame = [&y](Color c)
    { tft.drawRect(0, y - MENU_FRAME_TOP_OFFSET, SCREEN_LAST_X, MENU_FRAME_HEIGHT, c); };
    draw_row_frame(TFT_BLUE);

    while (true)
    {
        if (millis() - startMillis >= ROW_CHOICE_TIMEOUT_MS)
        {
            return row_index;
        }

        if (digitalRead(BUTTON_UP) == LOW)
        {
            draw_row_frame(TFT_BLACK);
            row_index = (row_index + 1) % rows.size();

            if (row_index)
            {
                y += row_height;
            }
            else
            {
                y = row_name_y_padding;
            }

            draw_row_frame(TFT_BLUE);
            delay(BUTTON_DEBOUNCE_MS);
        }
        if (digitalRead(BUTTON_DOWN) == LOW)
        {
            delay(BUTTON_DEBOUNCE_MS);
            return row_index;
        }
    }
}

void display_keyboard()
{
    tft.fillScreen(TFT_BLACK);
    for (int col = 0; col <= GRID_COLS; ++col)
    {
        int x = col * GRID_CELL_SIZE;
        tft.drawLine(x, GRID_TOP_Y, x, GRID_BOTTOM_Y + 1, TFT_WHITE);
    }

    for (int row = 0; row <= GRID_ROWS; ++row)
    {
        int y = GRID_TOP_Y + GRID_CELL_SIZE * row;
        tft.drawLine(0, y, SCREEN_WIDTH, y, TFT_WHITE);
    }

    tft.setTextSize(2);
    for (int row = 0; row < GRID_ROWS; ++row)
    {
        for (int col = 0; col < GRID_COLS; ++col)
        {
            const int key_index = col + row * GRID_COLS;
            if (key_index < static_cast<int>(KEYS.size()))
            {
                tft.setCursor(GRID_TEXT_OFFSET_X + GRID_CELL_SIZE * col,
                              GRID_TOP_Y + GRID_TEXT_OFFSET_Y + GRID_CELL_SIZE * row);
                String key = KEYS[key_index];
                key.toUpperCase();
                tft.print(key);
            }
        }
    }

    tft.fillRect(289, 140, 30, 29, TFT_BLACK);
    tft.fillRect(257, 140, 30, 29, TFT_BLACK);
    tft.fillRect(273, 154, 10, 1, TFT_WHITE);
    tft.fillTriangle(265, 154, 272, 150, 272, 158, TFT_WHITE);
    tft.fillRect(301, 155, 7, 8, TFT_WHITE);
    tft.fillTriangle(296, 155, 304, 147, 312, 155, TFT_WHITE);
}

String keyboard_input()
{
    display_keyboard();
    int selected_col = KEYBOARD_START_COL;
    int selected_row = KEYBOARD_START_ROW;
    tft.setCursor(INPUT_TEXT_X, INPUT_TEXT_Y);
    draw_keyboard_selector(selected_col, selected_row, TFT_BLUE);
    bool upper_letter = false;
    String password = "";
    unsigned long pressStartTime = 0;
    const unsigned long startMillis = millis();
    bool buttonPressed = false;

    while (true)
    {
        if (millis() - startMillis >= KEYBOARD_INPUT_TIMEOUT_MS)
        {
            return password;
        }

        if (digitalRead(BUTTON_UP) == LOW)
        {
            draw_keyboard_selector(selected_col, selected_row, TFT_WHITE);
            ++selected_col;
            if (selected_col >= GRID_COLS)
            {
                selected_col = 0;
                ++selected_row;
                if (selected_row >= GRID_ROWS)
                {
                    selected_row = 0;
                }
            }
            draw_keyboard_selector(selected_col, selected_row, TFT_BLUE);
            delay(KEYBOARD_NAV_DELAY_MS);
        }

        if (digitalRead(BUTTON_DOWN) == LOW)
        {
            if (!buttonPressed)
            {
                pressStartTime = millis();
                buttonPressed = true;
            }
        }
        else
        {
            if (buttonPressed)
            {
                unsigned long pressDuration = millis() - pressStartTime;
                buttonPressed = false;

                if (pressDuration < HOLD_TIME)
                {
                    const int key_index = selected_col + (selected_row * GRID_COLS);
                    if (key_index < static_cast<int>(KEYS.size()))
                    {
                        if (upper_letter)
                        {
                            String key = KEYS[key_index];
                            key.toUpperCase();
                            tft.print(key);
                            password += key;
                            upper_letter = false;
                        }
                        else
                        {
                            tft.print(KEYS[key_index]);
                            password += KEYS[key_index];
                        }
                    }
                    else if (key_index == SHIFT_KEY_INDEX)
                    {
                        upper_letter = true;
                    }
                    else if (key_index == BACKSPACE_KEY_INDEX)
                    {
                        tft.fillRect(0, 0, SCREEN_WIDTH, INPUT_CLEAR_HEIGHT, TFT_BLACK);
                        if (!password.isEmpty())
                        {
                            password = password.substring(0, password.length() - 1);
                        }
                        tft.setCursor(INPUT_TEXT_X, INPUT_TEXT_Y);
                        tft.print(password);
                    }
                }
                else
                {
                    return password;
                }
            }
            delay(KEYBOARD_NAV_DELAY_MS);
        }
    }
}