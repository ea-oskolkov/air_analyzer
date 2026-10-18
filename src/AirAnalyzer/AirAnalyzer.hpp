#pragma once

#include "log.hpp"
#include "utils/math.hpp"

#include "types.hpp"
#include "Renderer/Renderer.hpp"
#include "HTTPServer/HTTPServer.hpp"
#include "SensorsReader/SensorsReader.hpp"

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <stdint.h>

class AirAnalyzer final
{
public:
    using MilliSeconds = unsigned long;
    struct Conf
    {
        MilliSeconds update_backlight_interval = 0;
        MilliSeconds check_wifi_status_interval = 0;
        MilliSeconds update_data_interval = 0;

        const char* wifi_ssid = nullptr;
        const char* wifi_password = nullptr;

        uint16_t http_server_port = 80;

        uint8_t dht_type = 21;
        uint8_t dht_pin  = 0;

        uint8_t ds18b20_pin = 0;
        uint8_t ps_pin = 0;

        uint8_t tft_pin_cs;
        uint8_t tft_pin_dc;
        uint8_t tft_led_pin;

        uint8_t mhz19_pwm_pin = 0;
    };

    static AirAnalyzer& init(const Conf& conf);
    static AirAnalyzer& get_instance();

    ~AirAnalyzer() = default;
    AirAnalyzer(const AirAnalyzer&) = delete;
    AirAnalyzer& operator=(const AirAnalyzer&) = delete;
    AirAnalyzer(AirAnalyzer&&) = delete;
    AirAnalyzer& operator=(AirAnalyzer&&) = delete;

    /** @brief Обработчик рабочего цикла */
    void iteration();

private:
    static constexpr int m_conf_light_threshold_ps_val = 100;

    Conf m_conf;
    Renderer m_renderer;
    HTTPServer m_http_server;
    SensorsReader& m_sensors_reader;

    MilliSeconds m_prev_time_update_backlight = 0;
    MilliSeconds m_prev_time_check_wifi_status = 0;
    MilliSeconds m_prev_time_update_data = 0;

    bool network = false;
    SensorsData m_sd;

    AirAnalyzer(const AirAnalyzer::Conf& conf);

    /** @brief Логирование данных с датчиков */
    static void P_log_sd(const SensorsData& sd);

    static void P_backlight_init(Renderer& renderer, int ps_pin);

    /** @brief Управление уровнем подсветки на основе данных с фоторезистора. */
    static void P_backlight_adaptation(Renderer& renderer, int ps_pin);

    /** @brief Обновить данные на потребителях. */
    static void P_update_all_data(
        HTTPServer& http_server,
        Renderer& renderer,
        const SensorsData& sd,
        bool network
    );

    /** @brief Подключение к Wi-Fi */
    static void P_connect_to_wifi(const char* ssid, const char* password);

    /** @retval true   Подключен к WiFi */
    static bool P_wifi_status(bool was_connected);
};
