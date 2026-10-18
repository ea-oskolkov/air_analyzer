#include "AirAnalyzer/AirAnalyzer.hpp"

#include "secrets.inc"

void setup()
{
    static const AirAnalyzer::Conf conf =
    {
        .update_backlight_interval = 500,
        .check_wifi_status_interval = 500,
        .update_data_interval = (10 * 1000),

        .wifi_ssid     = SECRETS_WIFI_SSID,
        .wifi_password = SECRETS_WIFI_PASS,

        .http_server_port = 80,

        .dht_type = 21,
        .dht_pin  = D6,

        .ds18b20_pin = D4,
        .ps_pin = A0,

        .tft_pin_cs  = D0,
        .tft_pin_dc  = D8,
        .tft_led_pin = D3,

        .mhz19_pwm_pin = D1
    };

    AirAnalyzer::init(conf);
}

void loop()
{
    AirAnalyzer& air_analyzer = AirAnalyzer::get_instance();
    air_analyzer.iteration();
}
