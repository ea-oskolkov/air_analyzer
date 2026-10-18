#include "SensorsReader/SensorsReader.hpp"
#include "log.hpp"
#include "static_conf.hpp"
#include "utils/math.hpp"
#include "mhz19.inc"

SensorsReader* instance = nullptr;

/** @brief Устанавливает значение invalid_value, если значение является
 *         нечисловым значением или значение бесконечно.
 * */
static float P_sensor_value_float(float v)
{ return (Utils::Math::is_valid_float(v) ? v : invalid_sensor_value); }

SensorsReader& SensorsReader::init(
    uint8_t dht_pin,
    uint8_t dht_type,
    uint8_t ds18b20_pin,
    uint8_t mhz19_pwm_pin
)
{
    static SensorsReader inst(
        dht_pin,
        dht_type,
        ds18b20_pin,
        mhz19_pwm_pin
    );
    return inst;
}

SensorsReader& get_instance()
{ return *instance; }

SensorsReader::SensorsReader(
    uint8_t dht_pin,
    uint8_t dht_type,
    uint8_t ds18b20_pin,
    uint8_t mhz19_pwm_pin
)
: m_dht(dht_pin, dht_type)
, m_one_wire(ds18b20_pin)
, m_ds18b20(&m_one_wire)
, m_mhz19_pwm_pin(mhz19_pwm_pin)
{ instance = this; }

bool SensorsReader::init_sensors()
{
    m_ds18b20.setWaitForConversion(false);
    m_ds18b20.begin();

    DeviceAddress address;
    if (m_ds18b20.getDeviceCount() == 0)
    {
        Log::println("DS18B20: no devices found");
        return false;
    }

    if (!m_ds18b20.getAddress(address, 0))
    {
        Log::println("DS18B20: unable to get address of first sensor");
        return false;
    }

    if (!m_ds18b20.isConnected(address))
    {
        Log::println("DS18B20: sensor is not responding");
        return false;
    }

    m_dht.begin();

    pinMode(m_mhz19_pwm_pin, INPUT_PULLUP);
    attachInterrupt(
        digitalPinToInterrupt(m_mhz19_pwm_pin),
        P_interrupt_handler_mhz19,
        CHANGE
    );

    if (!m_ds18b20.requestTemperatures())
    {
        Log::println("ds18b20 request failure");
        return false;
    }
    delay(MAX_CONVERSION_TIMEOUT);

    return true;
}

SensorsData SensorsReader::read(MilliSeconds now)
{
    SensorsData data;
    data.temp = P_sensor_value_float(m_dht.readTemperature());
    data.humidity = P_sensor_value_float(m_dht.readHumidity());
    data.external_temp = P_read_external_temp(now, m_prev_external_temp);
    data.co2 = P_read_co2_ppm();
    m_prev_external_temp = data.external_temp;
    return data;
}

float SensorsReader::P_read_external_temp(
    MilliSeconds now,
    float prev_value
)
{
    const auto time_diff = now - m_prev_time_request_external_temp;
    const bool timeout = time_diff > MAX_CONVERSION_TIMEOUT;
    const bool conversion_complete = m_ds18b20.isConversionComplete();

    if (!timeout && !conversion_complete)
    {
        Log::println("ds18b20 return prev value");
        return prev_value;
    }

    if (timeout)
    {
        m_prev_time_request_external_temp = now;
        bool error = false;

        if (!conversion_complete)
        {
            Log::println("ds18b20 timeout");
            error = true;
        }

        if (!m_ds18b20.requestTemperatures())
        {
            Log::println("ds18b20 request failure");
            error = true;
        }

        if (error)
        {
            return invalid_sensor_value;
        }
    }

    const auto raw_value = m_ds18b20.getTempCByIndex(0);
    if (
        raw_value == DEVICE_DISCONNECTED_F ||
        raw_value == DEVICE_FAULT_OPEN_F ||
        raw_value == DEVICE_FAULT_SHORTGND_F ||
        raw_value == DEVICE_FAULT_SHORTVDD_F
    )
    {
        Log::println("ds18b20 error");
        return invalid_sensor_value;
    }

    return P_sensor_value_float(raw_value);
}

float SensorsReader::P_read_co2_ppm()
{
    noInterrupts();
    const MilliSeconds duration_high = m_mhz19.duration_high;
    const MilliSeconds duration_cycle = m_mhz19.duration_cycle;
    const MilliSeconds last_update_time = m_mhz19.last_update_time;
    interrupts();

    const MilliSeconds now = millis();
    if (now - last_update_time > m_conf_timeout_mhz19)
    {
        Log::println("MHZ19: no response");
        return invalid_sensor_value;
    }

    const bool valid_values =
        duration_high >= 2 &&
        duration_cycle > 4 &&
        duration_high <= duration_cycle &&
        duration_cycle - duration_high >= 2;

    if (!valid_values)
    {
        Log::println("MHZ19: invalid values");
        return invalid_sensor_value;
    }

    return (
        MHZ19_MAX_PPM * static_cast<float>(duration_high - 2) /
        static_cast<float>(duration_cycle - 4)
    );
}

void SensorsReader::P_interrupt_handler_mhz19()
{
    SensorsReader& reader = *instance;
    const MilliSeconds now = millis();

    if (digitalRead(reader.m_mhz19_pwm_pin) == LOW)
    {
        reader.m_mhz19.falling_timepoint = now;
        if (reader.m_mhz19.rising_timepoint != 0)
        {
            reader.m_mhz19.duration_high =
                now - reader.m_mhz19.rising_timepoint;
        }
    }
    else
    {
        if (reader.m_mhz19.falling_timepoint != 0)
        {
            reader.m_mhz19.duration_cycle =
                reader.m_mhz19.duration_high +
                (now - reader.m_mhz19.falling_timepoint);
            reader.m_mhz19.falling_timepoint = 0;
            reader.m_mhz19.last_update_time = now;
        }

        reader.m_mhz19.rising_timepoint = now;
    }
}
