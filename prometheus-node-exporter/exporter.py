import time
import logging
import requests
import argparse

from prometheus_client import start_http_server, Gauge

POLL_INTERVAL = 60

logging.basicConfig(level=logging.INFO)

metrics = {}
scrape_success = Gauge(
    "device_scrape_success",
    "Whether the last device poll succeeded"
)

def update_metrics(data, prefix="aa"):
    for key, value in data.items():
        name = f"{prefix}_{key}".lower()
        name = "".join(
            c if c.isalnum() or c == "_" else "_"
            for c in name
        )

        if isinstance(value, dict):
            update_metrics(value, name)

        elif isinstance(value, (int, float)) and not isinstance(value, bool):
            if name not in metrics:
                metrics[name] = Gauge(
                    name,
                    f"Device metric: {name}"
                )

            metrics[name].set(value)

def collect(url):
    try:
        response = requests.get(url, timeout=10)
        response.raise_for_status()

        data = response.json()

        if not isinstance(data, dict):
            raise ValueError("Expected a JSON object")

        update_metrics(data)
        scrape_success.set(1)

        logging.info("Device metrics updated")

    except Exception as error:
        scrape_success.set(0)
        logging.error("Device polling failed: %s", error)

def main():
    parser = argparse.ArgumentParser(description='Node exporter for AirAnalyzer')
    parser.add_argument("url", help="URL for AirAnalyzer")
    parser.add_argument("port", help="listening port", type=int)
    args = parser.parse_args()

    listening_port = args.port
    start_http_server(listening_port)
    logging.info(f"Exporter listening on port {listening_port}")

    while True:
        collect(args.url)
        time.sleep(POLL_INTERVAL)


if __name__ == "__main__":
    main()
