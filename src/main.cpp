#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <math.h>
#include "config.h"
#include "web_page.h"

struct SensorData {
    float currentRms;
    float currentPeak;
    float activePower;
    float apparentPower;
    float powerFactor;
    float energyKwh;
    uint32_t uptimeSec;
    bool overCurrent;
};

static Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
static WebServer server(80);
static WiFiManager wifiManager;

static SensorData g_data = {};
static uint32_t g_lastSample   = 0;
static uint32_t g_lastDisplay  = 0;
static uint32_t g_lastEnergy   = 0;
static uint32_t g_sessionStart = 0;

static void displayMessage(const char *line1, const char *line2 = nullptr, const char *line3 = nullptr) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(line1);
    if (line2) { display.setCursor(0, 14); display.print(line2); }
    if (line3) { display.setCursor(0, 28); display.print(line3); }
    display.display();
}

static float readCurrentRms(float &peakOut) {
    float sumSq = 0.0f;
    float peak  = 0.0f;
    for (int i = 0; i < ACS_SAMPLES; i++) {
        int raw = analogRead(ACS_PIN);
        float voltage = (raw / ACS_ADC_RES) * ACS_VREF;
        float current = (voltage - ACS_MIDPOINT) / ACS_SENSITIVITY;
        float absCurrent = fabsf(current);
        sumSq += current * current;
        if (absCurrent > peak) peak = absCurrent;
        delayMicroseconds(100);
    }
    peakOut = peak;
    return sqrtf(sumSq / ACS_SAMPLES);
}

static void updateDisplay(const SensorData &d) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(F("POWER MONITOR"));

    IPAddress ip = WiFi.localIP();
    if (ip[0] != 0) {
        display.setCursor(72, 0);
        char ipbuf[16];
        snprintf(ipbuf, sizeof(ipbuf), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
        display.print(ipbuf);
    }

    display.drawLine(0, 10, OLED_WIDTH - 1, 10, SSD1306_WHITE);

    display.setCursor(0, 14);
    display.print(F("I:"));
    display.setTextSize(2);
    display.setCursor(16, 12);
    char buf[12];
    snprintf(buf, sizeof(buf), "%5.2fA", d.currentRms);
    display.print(buf);

    display.setTextSize(1);
    display.setCursor(0, 32);
    display.print(F("P:"));
    snprintf(buf, sizeof(buf), "%6.1fW", d.activePower);
    display.print(buf);

    display.setCursor(0, 42);
    display.print(F("S:"));
    snprintf(buf, sizeof(buf), "%6.1fVA", d.apparentPower);
    display.print(buf);

    display.setCursor(0, 52);
    display.print(F("E:"));
    snprintf(buf, sizeof(buf), "%.4fkWh", d.energyKwh);
    display.print(buf);

    if (d.overCurrent) {
        display.fillRect(0, 0, OLED_WIDTH, OLED_HEIGHT, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setTextSize(2);
        display.setCursor(8, 20);
        display.print(F("OVERCURR"));
        display.setCursor(8, 40);
        snprintf(buf, sizeof(buf), "%.1fA!", d.currentRms);
        display.print(buf);
        display.setTextColor(SSD1306_WHITE);
    }

    display.display();
}

static void handleRoot() {
    server.sendHeader("Cache-Control", "no-cache");
    server.send_P(200, "text/html", INDEX_HTML);
}

static void handleData() {
    JsonDocument doc;
    doc["current_rms"]  = serialized(String(g_data.currentRms,  4));
    doc["current_peak"] = serialized(String(g_data.currentPeak, 4));
    doc["power_w"]      = serialized(String(g_data.activePower,  2));
    doc["apparent_va"]  = serialized(String(g_data.apparentPower, 2));
    doc["power_factor"] = serialized(String(g_data.powerFactor,  4));
    doc["energy_kwh"]   = serialized(String(g_data.energyKwh,    6));
    doc["uptime_sec"]   = g_data.uptimeSec;
    doc["overcurrent"]  = g_data.overCurrent;

    String out;
    out.reserve(256);
    serializeJson(doc, out);

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(200, "application/json", out);
}

static void handleNotFound() {
    server.send(404, "text/plain", "Not Found");
}

void setup() {
    Serial.begin(115200);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    pinMode(ACS_PIN, INPUT);

    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        while (true) { delay(1000); }
    }

    displayMessage("POWER MONITOR", "Starting...");

    wifiManager.setDebugOutput(false);
    wifiManager.setConnectTimeout(30);
    wifiManager.setConfigPortalTimeout(WM_TIMEOUT_SEC);

    wifiManager.setAPCallback([](WiFiManager *wm) {
        displayMessage(
            "WiFi Setup Mode",
            "SSID: " WM_AP_NAME,
            "Pass: " WM_AP_PASSWORD
        );
    });

    wifiManager.setSaveConfigCallback([]() {
        displayMessage("WiFi Saved", "Connecting...");
    });

    bool connected = wifiManager.autoConnect(WM_AP_NAME, WM_AP_PASSWORD);

    if (connected) {
        char ssidBuf[33];
        strncpy(ssidBuf, WiFi.SSID().c_str(), sizeof(ssidBuf) - 1);
        ssidBuf[sizeof(ssidBuf) - 1] = '\0';
        displayMessage("WiFi Connected", ssidBuf, WiFi.localIP().toString().c_str());
    } else {
        displayMessage("WiFi Failed", "Retrying next", "power cycle.");
    }

    delay(1500);

    server.on("/",      HTTP_GET, handleRoot);
    server.on("/data",  HTTP_GET, handleData);
    server.on("/reset", HTTP_GET, []() {
        server.send(200, "text/plain", "Resetting WiFi credentials...");
        delay(500);
        wifiManager.resetSettings();
        ESP.restart();
    });
    server.onNotFound(handleNotFound);
    server.begin();

    g_sessionStart = millis();
    g_lastSample   = millis();
    g_lastDisplay  = millis();
    g_lastEnergy   = millis();
}

void loop() {
    server.handleClient();

    uint32_t now = millis();

    if (now - g_lastSample >= SAMPLE_INTERVAL_MS) {
        g_lastSample = now;

        float peak = 0.0f;
        float rms  = readCurrentRms(peak);

        g_data.currentRms    = rms;
        g_data.currentPeak   = peak;
        g_data.activePower   = rms * MAINS_VOLTAGE;
        g_data.apparentPower = (peak * 0.7071f) * MAINS_VOLTAGE;
        g_data.powerFactor   = (g_data.apparentPower > 0.01f)
                                 ? constrain(g_data.activePower / g_data.apparentPower, 0.0f, 1.0f)
                                 : 0.0f;
        g_data.overCurrent   = (rms >= ACS_OVERCURRENT);
        g_data.uptimeSec     = (now - g_sessionStart) / 1000;
    }

    if (now - g_lastEnergy >= ENERGY_INTERVAL_MS) {
        float dt = (now - g_lastEnergy) / 3600000.0f;
        g_lastEnergy = now;
        g_data.energyKwh += (g_data.activePower / 1000.0f) * dt;
    }

    if (now - g_lastDisplay >= DISPLAY_INTERVAL_MS) {
        g_lastDisplay = now;
        updateDisplay(g_data);
    }
}