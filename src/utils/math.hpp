#pragma once

#include <math.h>

namespace Utils
{
namespace Math
{

/** @retval true Число v входит в [min, max]. */
template<typename Type>
constexpr bool check_in_range(Type v, Type min, Type max)
{ return (v >= min && v <= max); }


/** @retval true если значение является числовым и конечным. */
static inline bool is_valid_float(float v)
{ return (!isnan(v) && isfinite(v)); }

}
}
