#pragma once

#include <stdint.h>

struct SensorsData
{
    float temp = 0;
    float humidity = 0;
    float external_temp = 0;
    float co2 = 0;
};

struct NormalRangeData
{
    float max_co2_ppm = 800.0;

    float min_temp = 22.0;
    float max_temp = 26.0;

    float min_humidity = 28.0;
    float max_humidity = 50.0;
};

