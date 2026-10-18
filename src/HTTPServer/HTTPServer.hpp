#pragma once

#include "types.hpp"
#include "log.hpp"

#include <Arduino.h>
#include <ESP8266WebServer.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>

/** @brief HTTP-сервер */
class HTTPServer final
{
public:
    explicit HTTPServer(uint16_t port)
    : m_web_server(port)
    {
        json_buff[0] = '\0';
        m_web_server.on("/", [this]{P_handler();});
        m_web_server.begin();
    }

    /** @brief Обновить данные */
    void update_data(const SensorsData& sensors_data)
    {
        static constexpr char pattern[] =
            "{\"t\":%0.2f,\"h\":%0.2f,\"c\":%0.2f,\"t2\":%0.2f}";
        static constexpr size_t size = sizeof(json_buff);
        const int res = snprintf(
            json_buff,
            size,
            pattern,
            sensors_data.temp,
            sensors_data.humidity,
            sensors_data.co2,
            sensors_data.external_temp
        );

        if (res < 0)
        {
            const int err = errno;
            Log::print("HTTPServer: error creating JSON: ");
            Log::println(err);
        }
        else if (static_cast<size_t>(res) >= size)
        {
            Log::println("HTTPServer: buffer too small");
        }
    }

    /** @brief Обработка пользовательских запросов */
    void maintain()
    { m_web_server.handleClient(); }

private:
    ESP8266WebServer m_web_server;
    char json_buff[64];

    void P_handler()
    {
        static constexpr int code_http_ok = 200;
        m_web_server.sendHeader("Access-Control-Allow-Origin", "*");
        m_web_server.send(code_http_ok, "application/json", json_buff);
    }
};
