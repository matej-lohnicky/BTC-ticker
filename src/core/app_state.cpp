#include <TFT_eSPI.h>
#include <modules/app_state.h>

#include <vector>

DisplayMode mode = DisplayMode::Bitcoin;
bool changed_mode = true;

int angle = 0;
int angleMax = 30;
int angleMin = -5;
bool reversal = true;

byte currentBrightness = 120;

int pointRadius = 3;
int curveColor = TFT_BLUE;
int price = 0;
int chartTime = 0;
double percentChange = 0.0;
int chartTimeChange = 0;
bool priceFreshSample = false;
std::vector<int> readings;

int globalMinutes = 0;
int globalHours = 0;
int globalDay = 0;

int weatherCode = 0;
int sunriseHours = 0;
int sunriseMinutes = 0;
int sunsetHours = 0;
int sunsetMinutes = 0;
float currentTemperature = 0.0F;
std::vector<int> precipitationProbability;
