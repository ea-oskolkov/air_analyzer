#pragma once

#include "types.hpp"

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#include <stdint.h>

/** @brief Отрисовка данных на TFT-экран */
class Renderer final
{
public:
    enum class Backlight
    {
        LIGHT,
        DARK
    };

    Renderer(
        int8_t pin_cs,
        int8_t pin_dc,
        uint8_t pin_led,
        int precision,
        const NormalRangeData& normal_range_data = {}
    )
    : m_conf_tft_led_pin(pin_led)
    , m_conf_precision(precision)
    , m_conf_normal_range_data(normal_range_data)
    , m_tft(pin_cs, pin_dc)
    {
        pinMode(m_conf_tft_led_pin, OUTPUT);
        m_tft.begin();
        m_tft.setRotation(3);
    }

    void update_data(const SensorsData& sd, bool network);

    void set_backlight(Backlight backlight) const
    { analogWrite(m_conf_tft_led_pin, (backlight == Backlight::DARK ? 32 : 255)); }

private:
    static constexpr int m_conf_temp_y = 8;
    static constexpr int m_conf_humidity_y = 56;
    static constexpr int m_conf_co2_y = 104;
    static constexpr int m_conf_external_temp_y = 152;
    static constexpr int m_conf_network_y = 216;

    static constexpr int m_conf_sensor_row_height = 48;
    static constexpr int m_conf_network_row_height = 24;

    const int m_conf_tft_led_pin;
    const int m_conf_precision;
    const NormalRangeData m_conf_normal_range_data;
    Adafruit_ILI9341 m_tft;
    SensorsData m_prev_sd{};
    bool m_has_prev_sd = false;
    bool m_prev_network = false;

    void P_draw_temperature(float value);
    void P_draw_humidity(float value);
    void P_draw_co2(float value);
    void P_draw_external_temperature(float value);
    void P_draw_network(bool network);
    static void P_tft_print_degrees_circle(Adafruit_ILI9341& tft, uint16_t color);
};
