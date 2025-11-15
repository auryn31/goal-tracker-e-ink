#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <esp_wifi.h>
#include "GoalData.h"
#include "config.h"

class NetworkManager {
public:
    static bool connectWiFi();
    static void syncTime();
    static String formatTimestamp(const char* isoTimestamp);
    static String getTargetDate(int daysToGoal);
    static bool fetchGoalData(GoalData& data);
    static bool fetchMockData(GoalData& data);
};

#endif // NETWORK_MANAGER_H
