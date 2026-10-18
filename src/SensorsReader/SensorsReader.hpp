#pragma once

#include "types.hpp"

#include <Arduino.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class SensorsReader final
{
public:
    using MilliSeconds = unsigned long;

    static SensorsReader& init(
        uint8_t dht_pin,
        uint8_t dht_type,
        uint8_t ds18b20_pin,
        uint8_t mhz19_pwm_pin
    );

    static SensorsReader& get_instance();

    bool init_sensors();
    SensorsData read(MilliSeconds now);

private:
    struct Mhz19
    {
        volatile MilliSeconds rising_timepoint  = 0;
        volatile MilliSeconds falling_timepoint = 0;
        volatile MilliSeconds duration_high     = 0;
        volatile MilliSeconds duration_cycle    = 0;
        volatile MilliSeconds last_update_time  = 0;
    };

    static constexpr MilliSeconds m_conf_timeout_mhz19 = (1004 * 2);

    DHT m_dht;
    OneWire m_one_wire;
    DallasTemperature m_ds18b20;
    const uint8_t m_mhz19_pwm_pin;
    Mhz19 m_mhz19{};

    MilliSeconds m_prev_time_request_external_temp = 0;
    float m_prev_external_temp = 0;

    SensorsReader(
        uint8_t dht_pin,
        uint8_t dht_type,
        uint8_t ds18b20_pin,
        uint8_t mhz19_pwm_pin
    );

    static IRAM_ATTR void P_interrupt_handler_mhz19();
    float P_read_external_temp(
        MilliSeconds now,
        float prev_value
    );
    float P_read_co2_ppm();
};
