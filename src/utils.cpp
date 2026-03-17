#include <TFT_eSPI.h>
#include <modules/constants.h>
#include <modules/sprites.h>
#include <modules/variables.h>

#include <vector>

const std::vector<String> KEYS = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "a", "b", "c",
                                  "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p",
                                  "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", ".", "+"};

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

    auto draw_row_frame = [&y](Color c) { tft.drawRect(0, y - 3, 319, 21, c); };
    draw_row_frame(TFT_BLUE);

    while (true)
    {
        if (digitalRead(BUTTON_UP) == LOW)
        {  // browsing through the networks
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
            delay(250);  // KEEP SOME DELAY, for the buttons to work properly
        }
        if (digitalRead(BUTTON_DOWN) == LOW)
        {  // choosing the network
            delay(250);
            return row_index;
        }
    }
}

void display_keyboard()
{
    // frame
    tft.fillScreen(TFT_BLACK);
    for (int y = 0; y < 11; y++)
    {  // vertical
        tft.drawLine(32 * y, 42, 32 * y, 170, TFT_WHITE);
    }
    tft.drawLine(319, 42, 319, 170, TFT_WHITE);

    for (int x = 0; x < 4; x++)
    {  // horizontal
        tft.drawLine(0, 42 + 32 * x, 320, 42 + 32 * x, TFT_WHITE);
    }
    tft.drawLine(0, 169, 320, 169, TFT_WHITE);

    // keys
    tft.setTextSize(2);
    int MaxIndex = KEYS.size();
    int KeyIndex = 0;
    for (int m = 0; m < 4; ++m)
    {
        for (int n = 0; n < 10; ++n)
        {
            tft.setCursor(12 + 32 * n, 51 + 32 * m);
            String key = KEYS[KeyIndex];
            key.toUpperCase();
            tft.print(key);
            KeyIndex = (KeyIndex + 1) % MaxIndex;
        }
    }

    // pictures for special keys
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
    int x = 0;
    int y = 32;
    tft.setCursor(5, 5);
    tft.drawRect(x, y + 42, 32 + 1, 32 + 1, TFT_BLUE);
    bool UpperLetter = false;
    String password = "";
    unsigned long pressStartTime = 0;
    bool buttonPressed = false;

    while (true)
    {
        if (digitalRead(BUTTON_UP) == LOW)
        {  // browsing through the keys
            tft.drawRect(x, y + 42, 32 + 1, 32 + 1, TFT_WHITE);
            x = x + 32;
            if (x >= 320)
            {
                x = 0;
                y = y + 32;
                if (y >= 128)
                {
                    y = 0;
                }
            }
            tft.drawRect(x, y + 42, 32 + 1, 32 + 1, TFT_BLUE);
            delay(100);  // KEEP SOME DELAY, for the buttons to work properly
        }

        if (digitalRead(BUTTON_DOWN) == LOW)
        {  // press for entering the key, hold for entering the whole
           // password
            if (!buttonPressed)
            {  // start timer when pressed
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
                    const int KeyIndex = (x / 32) + ((y / 32) * 10);
                    if (KeyIndex <= 37)
                    {
                        if (UpperLetter)
                        {
                            String key = KEYS[KeyIndex];
                            key.toUpperCase();
                            tft.print(key);
                            password += key;
                            UpperLetter = false;
                        }
                        else
                        {
                            tft.print(KEYS[KeyIndex]);
                            password += KEYS[KeyIndex];
                        }
                    }
                    else if (KeyIndex == 39)
                    {  // capslock
                        UpperLetter = true;
                    }
                    else if (KeyIndex == 38)
                    {  // delete
                        tft.fillRect(0, 0, 320, 40, TFT_BLACK);
                        if (!password.isEmpty())
                        {
                            password = password.substring(0, password.length() - 1);
                        }
                        tft.setCursor(5, 5);
                        tft.print(password);
                    }
                }
                else
                {  // buton held
                    return password;
                }
            }
            delay(100);  // KEEP SOME DELAY, for the buttons to work properly
        }
    }
}