#pragma once

class String;

using byte = unsigned char;

namespace std
{
template <typename T>
class allocator;

template <typename T, typename Allocator>
class vector;
}  // namespace std

enum class DisplayMode : int
{
    Bitcoin = 0,
    Clock = 1,
    Weather = 2,
};

extern DisplayMode mode;
extern bool changed_mode;

extern int angle;
extern int angleMax;
extern int angleMin;
extern bool reversal;

extern byte currentBrightness;

extern int pointRadius;
extern int curveColor;
extern int price;
extern int chartTime;
extern double percentChange;
extern int chartTimeChange;
extern bool priceFreshSample;
extern std::vector<int, std::allocator<int>> readings;

extern int globalMinutes;
extern int globalHours;
extern int globalDay;

extern int weatherCode;
extern int sunriseHours;
extern int sunriseMinutes;
extern int sunsetHours;
extern int sunsetMinutes;
extern float currentTemperature;
extern std::vector<int, std::allocator<int>> precipitationProbability;
