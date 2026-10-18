#pragma once

#include <Arduino.h>

namespace Log
{

static inline void init()
{
#if BUILD_CONF__DEBUG_MODE
    Serial.begin(115200);
#endif
}

template <typename Type>
static void print([[maybe_unused]] Type msg)
{
#if BUILD_CONF__DEBUG_MODE
    Serial.print(msg);
#endif
}

template <typename Type>
static void println([[maybe_unused]] Type msg)
{
#if BUILD_CONF__DEBUG_MODE
    Serial.println(msg);
#endif
}

}
