#  Smart IoT Poultry Scale & Trade Logger

An industry-ready, edge IoT weighing system built for poultry farm management. Designed with **ESP32-S3**, **HX711 load cell amplifier**, **MQTT (HiveMQ WebSockets)**, **Google Apps Script**, and **Telegram Bot Integration**.

---

##  Key Features

* **Real-Time Telemetry:** Streams continuous weight readings to a web dashboard over secure MQTT (`wss://`).
* **Cloud Trade Logging:** Appends trade weight, date, time, scale ID, and location to **Google Sheets** with one click.
* **Instant Receipts:** Dispatches structured trade receipts to a **Telegram** group/chat automatically.
* **GitHub OTA Updates:** Remote firmware update support via `HTTPUpdate` from GitHub.
* **On-Site QR Access:** Dynamic QR code on dashboard for fast mobile access to live trade logs.

---

##  System Architecture

```text
[ Load Cell + HX711 ]
          │
          ▼
    [ ESP32-S3 ] ──────(MQTT WebSockets)─────► [ Web Dashboard ]
          │                                         │
     (WiFi/HTTP)                               (POST Request)
          │                                         │
          ▼                                         ▼
   [ GitHub OTA ]                          [ Google Apps Script ]
                                                    │
                                           ┌────────┴────────┐
                                           ▼                 ▼
                                   [ Google Sheets ]  [ Telegram Bot ]
##  Authors & License

* **Developed by:** Pranav Sunil Gholap
* **License:** Open-Source under the [MIT License](LICENSE)
