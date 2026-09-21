#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "HX711.h"

// --- Current Firmware Version ---
const String CURRENT_VERSION = "1.0.0"; 

// --- GitHub OTA URLs (Replace with your actual GitHub username and Repo) ---
// Note: Use raw.githubusercontent.com for direct file downloads!
const char* version_url  = "https://raw.githubusercontent.com/pranavgholap2111-sys/Weighing_Scale/main/version.txt";
const char* firmware_url = "https://raw.githubusercontent.com/pranavgholap2111-sys/Weighing_Scale/main/firmware.bin";

// --- Hardware Pins ---
#define HX711_DOUT_PIN 1
#define HX711_SCK_PIN  2
#define LCD_SDA_PIN    21
#define LCD_SCL_PIN    47

// --- MQTT & Scale Settings ---
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "smartscale/telemetry/weight";
float calibration_factor = 420.0;

HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsgTime = 0;

void checkForOTAUpdate() {
  if (WiFi.status() != WL_CONNECTED) return;

  Serial.println("\n--- Checking for GitHub OTA Updates ---");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Checking OTA...");

  WiFiClientSecure clientSecure;
  clientSecure.setInsecure(); // Bypass SSL certificate verification for GitHub HTTPS

  HTTPClient http;
  http.begin(clientSecure, version_url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    payload.trim(); // Get version string from version.txt
    
    Serial.print("Current Firmware Version: ");
    Serial.println(CURRENT_VERSION);
    Serial.print("GitHub Firmware Version:  ");
    Serial.println(payload);

    if (payload != CURRENT_VERSION) {
      Serial.println("New firmware version detected! Starting OTA update...");
      lcd.setCursor(0, 0);
      lcd.print("Updating Firmware");
      lcd.setCursor(0, 1);
      lcd.print("Do NOT Power Off!");

      // Execute OTA Flash
      t_httpUpdate_return ret = httpUpdate.update(clientSecure, firmware_url);

      switch (ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("OTA Update Failed. Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
          break;

        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("No updates found.");
          break;

        case HTTP_UPDATE_OK:
          Serial.println("OTA Update Successful! Rebooting...");
          break;
      }
    } else {
      Serial.println("Firmware is already up to date.");
      lcd.setCursor(0, 1);
      lcd.print("Version Up2Date!");
      delay(1000);
    }
  } else {
    Serial.printf("Failed to fetch version file. HTTP Code: %d\n", httpCode);
  }
  http.end();
}

void setup() {
  Serial.begin(115200);

  // 1. Wi-Fi Setup
  WiFiManager wm;
  wm.setConfigPortalTimeout(180);
  if (!wm.autoConnect("Poultry-Scale-AP")) {
    ESP.restart();
  }

  // 2. Hardware Setup
  pinMode(HX711_DOUT_PIN, INPUT_PULLUP);
  pinMode(HX711_SCK_PIN, OUTPUT);

  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
  lcd.init();
  lcd.backlight();

  // 3. Check GitHub for OTA updates before running main scale loop
  checkForOTAUpdate();

  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();

  client.setServer(mqtt_server, mqtt_port);
  lcd.clear();
}

void loop() {
  if (!client.connected()) {
    String clientId = "ESP32S3_Scale_";
    clientId += String(random(0xffff), HEX);
    client.connect(clientId.c_str());
  }
  client.loop();

  if (scale.is_ready()) {
    float grams = scale.get_units(3);

    lcd.setCursor(0, 0);
    lcd.print("Weight:         ");
    lcd.setCursor(0, 1);
    lcd.print(String(grams / 1000.0, 2) + " kg        ");

    if (millis() - lastMsgTime > 300) {
      lastMsgTime = millis();
      char weightStr[10];
      dtostrf(grams, 4, 1, weightStr);
      client.publish(mqtt_topic, weightStr);
    }
  }
}
