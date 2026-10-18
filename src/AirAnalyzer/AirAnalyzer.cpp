#include "AirAnalyzer/AirAnalyzer.hpp"
#include "static_conf.hpp"

using Backlight = Renderer::Backlight;

static AirAnalyzer* instance = nullptr;

static void P_emergency()
{ while (true) { delay(1000); } }

AirAnalyzer& AirAnalyzer::init(const Conf& conf)
{
    static AirAnalyzer inst(conf);
    return inst;
}

AirAnalyzer& AirAnalyzer::get_instance()
{ return *instance; }

AirAnalyzer::AirAnalyzer(
    const AirAnalyzer::Conf& conf
)
: m_conf(conf)
, m_renderer(m_conf.tft_pin_cs, m_conf.tft_pin_dc, m_conf.tft_led_pin, 0)
, m_http_server(m_conf.http_server_port)
, m_sensors_reader(SensorsReader::init(
    m_conf.dht_pin,
    m_conf.dht_type,
    m_conf.ds18b20_pin,
    m_conf.mhz19_pwm_pin
))
{
    instance = this;

    Log::init();
    Log::println("Init: begin");
    if (!m_sensors_reader.init_sensors())
    {
        Log::println("Init: failure");
        P_emergency();
    }

    P_backlight_init(m_renderer, m_conf.ps_pin);
    P_connect_to_wifi(m_conf.wifi_ssid, m_conf.wifi_password);

    {
        /* Первая итерация */
        const MilliSeconds now = millis();
        m_sd = m_sensors_reader.read(now);
        network = P_wifi_status(false);
        P_backlight_adaptation(m_renderer, m_conf.ps_pin);
        P_update_all_data(m_http_server, m_renderer, m_sd, network);
    }

    Log::println("Init: done");
}

void AirAnalyzer::iteration()
{
    const MilliSeconds now = millis();

    m_http_server.maintain();

    /* Задержка не учитывается, т.к не является значительной */
    if ((now - m_prev_time_update_backlight) >= m_conf.update_backlight_interval)
    {
        P_backlight_adaptation(m_renderer, m_conf.ps_pin);
        m_prev_time_update_backlight = now;
    }

    if ((now - m_prev_time_check_wifi_status) >= m_conf.check_wifi_status_interval)
    {
        const bool prev_network = network;
        network = P_wifi_status(prev_network);
        m_prev_time_check_wifi_status = now;
        m_renderer.update_data(m_sd, network);
    }

    if ((now - m_prev_time_update_data) >= m_conf.update_data_interval)
    {
        m_sd = m_sensors_reader.read(now);
        P_update_all_data(m_http_server, m_renderer, m_sd, network);
        m_prev_time_update_data = now;
    }
}


void AirAnalyzer::P_log_sd(const SensorsData& sd)
{
    Log::println("------- Sensors data -------");
    Log::print("Temp:");
    Log::println(sd.temp);

    Log::print("Humidity:");
    Log::println(sd.humidity);

    Log::print("co2:");
    Log::println(sd.co2);

    Log::print("External temp:");
    Log::println(sd.external_temp);

    Log::println("-----------------------------");
}

void AirAnalyzer::P_backlight_init(Renderer& renderer, int ps_pin)
{
    const int ps_val = analogRead(ps_pin);
    if (ps_val < m_conf_light_threshold_ps_val)
    {
        renderer.set_backlight(Backlight::DARK);
    }
    else
    {
        renderer.set_backlight(Backlight::LIGHT);
    }
}

void AirAnalyzer::P_backlight_adaptation(Renderer& renderer, int ps_pin)
{
    static constexpr int light_bounce_range_val = 15;

    const int ps_val = analogRead(ps_pin);
    if (ps_val < m_conf_light_threshold_ps_val)
    {
        renderer.set_backlight(Backlight::DARK);
    }
    else if (ps_val >= (m_conf_light_threshold_ps_val + light_bounce_range_val))
    {
        renderer.set_backlight(Backlight::LIGHT);
    }
    else
    {
        /* Do nothing */
    }
}

void AirAnalyzer::P_update_all_data(
    HTTPServer& http_server,
    Renderer& renderer,
    const SensorsData& sd,
    bool network
)
{
    http_server.update_data(sd);
    renderer.update_data(sd, network);
    P_log_sd(sd);
}

void AirAnalyzer::P_connect_to_wifi(const char* ssid, const char* password)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
}

bool AirAnalyzer::P_wifi_status(bool was_connected)
{
    const bool connected = (WiFi.status() == WL_CONNECTED);
    if (connected)
    {
        if (!was_connected)
        {
            Log::print("Successfully connected to Wi-Fi. IP: ");
            Log::println(WiFi.localIP());
        }
    }
    else
    {
        if (was_connected)
        {
            Log::println("Trying to connect to Wi-Fi...");
        }
    }
    return connected;
}
