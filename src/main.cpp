#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "GoalData.h"
#include "NetworkManager.h"
#include "DisplayManager.h"

// Sleep configuration
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP (UPDATE_INTERVAL / 1000)

void goToSleep() {
    Serial.println("\n---------------------------------");
    Serial.print("Going to deep sleep for ");
    Serial.print(TIME_TO_SLEEP / 60);
    Serial.println(" minutes...");
    Serial.println("---------------------------------");
    Serial.flush();

    // Power down display
    DisplayManager::hibernate();

    // Disconnect WiFi to save power
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    // Disable GPIO hold circuits to ensure pins don't stay active during deep sleep
    gpio_deep_sleep_hold_dis();

    delay(100);

    // Enter deep sleep
    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=================================");
    Serial.println("Goal Tracker - E-ink Display");
    Serial.print("Version: ");
    Serial.print(FIRMWARE_VERSION);
    Serial.print(" (");
    Serial.print(BUILD_DATE);
    Serial.print(" ");
    Serial.print(BUILD_TIME);
    Serial.println(")");
    Serial.println("=================================");

    // Configure deep sleep timer
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.print("Configured to sleep for: ");
    Serial.print(TIME_TO_SLEEP);
    Serial.println(" seconds");

    // Initialize display
    DisplayManager::init();

    // Create data structure
    GoalData data;
    data.isValid = false;
    data.lastUpdateSuccess = false;

    // Load cached data from RTC memory (survives deep sleep)
    if (DataStorage::hasData()) {
        Serial.println("Loading cached data from previous update...");
        DataStorage::load(data);
    }

    if (!NetworkManager::connectWiFi()) {
        Serial.println("WiFi connection failed!");
    }

    // Fetch goal data
    GoalData newData;
    if (NetworkManager::fetchGoalData(newData)) {
        // Success - update with fresh data (timestamp already set by fetchGoalData)
        newData.lastUpdateSuccess = true;
        newData.targetDate = NetworkManager::getTargetDate(newData.daysToGoal);

        // Save to RTC memory for next wake cycle
        DataStorage::save(newData);

        // Use the new data
        data = newData;

        Serial.println("Data fetched and cached successfully!");
    } else {
        // Failed - use cached data if available
        Serial.println("Error: Could not fetch data");

        if (DataStorage::hasData()) {
            Serial.println("Using cached data from previous update");
            data.lastUpdateSuccess = false;  // Mark as offline
        } else {
            Serial.println("No cached data available!");
            DisplayManager::showError("No data available");
            delay(3000);
            goToSleep();
            return;
        }
    }

    // Update display with current or cached data
    if (data.isValid) {
        DisplayManager::showGoalInfo(data);
        Serial.println("Display updated!");
    }

    // Go to deep sleep
    goToSleep();
}

void loop() {
    // Never reached due to deep sleep
    goToSleep();
}
