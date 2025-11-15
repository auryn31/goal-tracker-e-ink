#ifndef CONFIG_H
#define CONFIG_H

// ===== FIRMWARE VERSION =====
#define FIRMWARE_VERSION "1.0.0"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

// ===== WIFI CONFIGURATION =====
// Replace with your WiFi network name
constexpr const char* WIFI_SSID = "YOUR_WIFI_SSID";
// Replace with your WiFi password
constexpr const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Timezone offset in seconds
// Examples: -18000 (EST), -28800 (PST), 3600 (CET), 0 (UTC)
constexpr long TIMEZONE_OFFSET = 0;

// ===== API CONFIGURATION =====
// Replace with your server's IP address and port
constexpr const char* API_URL = "http://YOUR_SERVER_IP:4000/api/portfolio/summary";
// Replace with your API authorization token
constexpr const char* API_TOKEN = "YOUR_API_TOKEN_HERE";

// Mock mode for testing (set to true to use test data instead of real API)
// Useful for testing display without WiFi or API server running
constexpr bool MOCK_MODE = false;

// Debug mode - enable Serial output for debugging (set to false to save power and flash)
#define DEBUG_MODE true

// Debug macros
#if DEBUG_MODE
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
#endif

// ===== DISPLAY CONFIGURATION =====
// Update interval (in milliseconds) - device will deep sleep between updates
// Common intervals:
//   3600000 = 1 hour (recommended)
//   1800000 = 30 minutes
//   7200000 = 2 hours
//   300000 = 5 minutes (for testing only - reduces battery life)
constexpr unsigned long UPDATE_INTERVAL = 3600000; // 1 hour

// E-ink display pins (240x416 display) - ESP32-C3 Super Mini
// Based on WeAct Studio example: CS=7, SCK=4, MOSI=6, BUSY=3, RST=2, DC=1
// IMPORTANT: GPIO8 must be HIGH to power the display!
#define EPD_POWER_PIN 8   // GPIO8 - Display power enable (must be HIGH)
#define EPD_CS 7          // SS/CS - Chip Select
#define EPD_DC 1          // DC - Data/Command
#define EPD_RST 2         // RES/RST - Reset
#define EPD_BUSY 3        // BUSY - Busy signal

// SPI pins for ESP32-C3 Super Mini (hardware SPI)
#define EPD_MOSI 6        // SDA/MOSI - Data to display
#define EPD_SCK 4         // SCL/SCK - Clock

// Display rotation: 0 = portrait, 1 = landscape, 2 = portrait inverted, 3 = landscape inverted
#define DISPLAY_ROTATION 1

#endif // CONFIG_H
