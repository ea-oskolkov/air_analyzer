#include "Renderer/Renderer.hpp"
#include "utils/math.hpp"
#include "static_conf.hpp"

static float P_discard_sign_in_case_of_zero(float v, int precision)
{
    /* Чтобы избежать ситуации когда на экране будет -0 */
    if (precision == 0 && (v > -1.0 && v < 0.0))
    {
        return 0.0;
    }
    return v;
}

void Renderer::update_data(const SensorsData& sd, bool network)
{
    const bool update_all = !m_has_prev_sd;

    if (update_all)
    {
        m_tft.fillScreen(ILI9341_BLACK);
        P_draw_temperature(sd.temp);
        P_draw_humidity(sd.humidity);
        P_draw_co2(sd.co2);
        P_draw_external_temperature(sd.external_temp);
        P_draw_network(network);
    }
    else
    {
        if (sd.temp != m_prev_sd.temp)
        {
            P_draw_temperature(sd.temp);
        }
        if (sd.humidity != m_prev_sd.humidity)
        {
            P_draw_humidity(sd.humidity);
        }
        if (sd.co2 != m_prev_sd.co2)
        {
            P_draw_co2(sd.co2);
        }
        if (sd.external_temp != m_prev_sd.external_temp)
        {
            P_draw_external_temperature(sd.external_temp);
        }
        if (network != m_prev_network)
        {
            P_draw_network(network);
        }
    }

    m_prev_sd = sd;
    m_prev_network = network;
    m_has_prev_sd = true;
}

void Renderer::P_draw_temperature(float value)
{
    m_tft.fillRect(
        0,
        m_conf_temp_y,
        m_tft.width(),
        m_conf_sensor_row_height,
        ILI9341_BLACK
    );
    const bool normal = Utils::Math::check_in_range(
        value,
        m_conf_normal_range_data.min_temp,
        m_conf_normal_range_data.max_temp
    );
    const auto color_text = (normal ? ILI9341_GREEN : ILI9341_RED);
    m_tft.setCursor(4, m_conf_temp_y);
    m_tft.setTextSize(6);
    m_tft.setTextColor(color_text);
    const auto v = P_discard_sign_in_case_of_zero(value, m_conf_precision);
    m_tft.print(v, m_conf_precision);
    m_tft.print("  ");
    P_tft_print_degrees_circle(m_tft, color_text);
    m_tft.println("C");
}

void Renderer::P_draw_humidity(float value)
{
    m_tft.fillRect(
        0,
        m_conf_humidity_y,
        m_tft.width(),
        m_conf_sensor_row_height,
        ILI9341_BLACK
    );
    const bool normal = Utils::Math::check_in_range(
        value,
        m_conf_normal_range_data.min_humidity,
        m_conf_normal_range_data.max_humidity
    );
    m_tft.setCursor(4, m_conf_humidity_y);
    m_tft.setTextSize(6);
    m_tft.setTextColor(normal ? ILI9341_GREEN : ILI9341_RED);
    const auto v = P_discard_sign_in_case_of_zero(value, m_conf_precision);
    m_tft.print(v, m_conf_precision);
    m_tft.println("  %");
}

void Renderer::P_draw_co2(float value)
{
    m_tft.fillRect(
        0,
        m_conf_co2_y,
        m_tft.width(),
        m_conf_sensor_row_height,
        ILI9341_BLACK
    );
    const bool normal = Utils::Math::check_in_range(
        value,
        static_cast<float>(0.0),
        m_conf_normal_range_data.max_co2_ppm
    );
    m_tft.setCursor(4, m_conf_co2_y);
    m_tft.setTextSize(6);
    m_tft.setTextColor(normal ? ILI9341_GREEN : ILI9341_RED);
    const auto v = P_discard_sign_in_case_of_zero(value, m_conf_precision);
    m_tft.print(v, m_conf_precision);
    if (v < 1000)
    {
        m_tft.print(" ");
    }
    m_tft.setTextSize(4);
    m_tft.print("ppm");
}

void Renderer::P_draw_external_temperature(float value)
{
    m_tft.fillRect(
        0,
        m_conf_external_temp_y,
        m_tft.width(),
        m_conf_sensor_row_height,
        ILI9341_BLACK
    );
    const auto color_text = value == invalid_sensor_value
        ? ILI9341_RED
        : ILI9341_GREEN;
    m_tft.setCursor(4, m_conf_external_temp_y);
    m_tft.setTextSize(6);
    m_tft.setTextColor(color_text);
    const auto v = P_discard_sign_in_case_of_zero(value, m_conf_precision);
    m_tft.print(v, m_conf_precision);
    m_tft.print("  ");
    P_tft_print_degrees_circle(m_tft, color_text);
    m_tft.println("C");
}

void Renderer::P_draw_network(bool network)
{
    m_tft.fillRect(
        0,
        m_conf_network_y,
        m_tft.width(),
        m_conf_network_row_height,
        ILI9341_BLACK
    );
    m_tft.setCursor(4, m_conf_network_y);
    m_tft.setTextSize(2);
    m_tft.setTextColor(network ? ILI9341_GREEN : ILI9341_RED);
    m_tft.println(network ? "connected" : "disconnected");
}

void Renderer::P_tft_print_degrees_circle(Adafruit_ILI9341& tft, uint16_t color)
{
    tft.drawCircle(
        tft.getCursorX() + 5,
        tft.getCursorY() - 5,
        3,
        color
    );
}
