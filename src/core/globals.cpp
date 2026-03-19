#include <TFT_eSPI.h>

unsigned long lastRotationUpdate = 0;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite btc_logo = TFT_eSprite(&tft);
TFT_eSprite back_logo = TFT_eSprite(&tft);
TFT_eSprite btc_price = TFT_eSprite(&tft);
TFT_eSprite btc_percents = TFT_eSprite(&tft);
TFT_eSprite chart = TFT_eSprite(&tft);
TFT_eSprite sessions = TFT_eSprite(&tft);
TFT_eSprite clock_display = TFT_eSprite(&tft);
TFT_eSprite weatherIcon = TFT_eSprite(&tft);
TFT_eSprite temperature = TFT_eSprite(&tft);
TFT_eSprite temperatureExtremes = TFT_eSprite(&tft);
TFT_eSprite temperatureRange = TFT_eSprite(&tft);
TFT_eSprite rainChart = TFT_eSprite(&tft);