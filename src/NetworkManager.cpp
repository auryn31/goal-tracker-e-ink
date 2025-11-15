#include "NetworkManager.h"

bool NetworkManager::connectWiFi() {
    Serial.println("\n===== WiFi Connection =====");

    // Set station mode
    WiFi.mode(WIFI_STA);

    // Set maximum TX power for better signal strength
    // Range: 8-84 (2dBm - 21dBm), 40 = ~10dBm
    esp_wifi_set_max_tx_power(40);

    // Disable WiFi power saving for reliability
    WiFi.setSleep(WIFI_PS_NONE);

    // Set hostname
    WiFi.setHostname("ESP32-GoalTracker");

    delay(100);

    // Scan for target network
    Serial.println("Scanning for WiFi networks...");
    int n = WiFi.scanNetworks();
    Serial.print("Found ");
    Serial.print(n);
    Serial.println(" networks");

    // Find target network and get BSSID + channel
    int targetIndex = -1;
    for (int i = 0; i < n; i++) {
        if (WiFi.SSID(i) == String(WIFI_SSID)) {
            targetIndex = i;
            Serial.print("Target network found: ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.println(" dBm)");
            break;
        }
    }

    if (targetIndex < 0) {
        Serial.println("ERROR: Network not found!");
        return false;
    }

    // Get network details
    uint8_t* bssid = WiFi.BSSID(targetIndex);
    int32_t channel = WiFi.channel(targetIndex);

    Serial.print("BSSID: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", bssid[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
    Serial.print("Channel: ");
    Serial.println(channel);

    // Connect with BSSID + Channel for reliable connection
    Serial.println("\nConnecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, channel, bssid, true);

    // Wait for connection (20 seconds timeout)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        Serial.println("===========================\n");

        // Sync time with NTP server
        syncTime();
        return true;
    } else {
        Serial.println("\n✗ Connection failed!");
        return false;
    }
}

void NetworkManager::syncTime() {
    Serial.println("Syncing time with NTP server...");
    // Configure time with NTP (using timezone from config)
    configTime(TIMEZONE_OFFSET, 0, "pool.ntp.org", "time.nist.gov");

    // Wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (time(nullptr) < 100000 && ++retry < retry_count) {
        Serial.print(".");
        delay(500);
    }

    if (retry < retry_count) {
        Serial.println("\nTime synchronized!");
    } else {
        Serial.println("\nTime sync failed, using local time");
    }
}

String NetworkManager::formatTimestamp(const char* isoTimestamp) {
    // Parse ISO 8601 timestamp: "2025-10-28T11:51:53.666Z"
    if (!isoTimestamp || strlen(isoTimestamp) < 19) {
        return "N/A";
    }

    struct tm timeinfo = {0};
    int year, month, day, hour, minute, second;

    // Parse the ISO timestamp
    if (sscanf(isoTimestamp, "%d-%d-%dT%d:%d:%d",
               &year, &month, &day, &hour, &minute, &second) == 6) {

        timeinfo.tm_year = year - 1900;
        timeinfo.tm_mon = month - 1;
        timeinfo.tm_mday = day;
        timeinfo.tm_hour = hour;
        timeinfo.tm_min = minute;
        timeinfo.tm_sec = second;

        // Convert to local time
        time_t timestamp = mktime(&timeinfo);
        // Add timezone offset to convert from UTC
        timestamp += TIMEZONE_OFFSET;
        localtime_r(&timestamp, &timeinfo);

        // Format: "MM/DD HH:MM"
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%m/%d %H:%M", &timeinfo);
        return String(timeStr);
    }

    return "N/A";
}

String NetworkManager::getTargetDate(int daysToGoal) {
    time_t now = time(nullptr);
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo) || now < 100000) {
        // If time sync failed, return N/A instead of incorrect date
        return "N/A";
    }

    // Calculate target date by adding days to current date
    time_t targetTime = now + (daysToGoal * 86400); // 86400 seconds per day
    struct tm targetDate;
    localtime_r(&targetTime, &targetDate);

    // Format: "Day, Mon DD, YYYY" (e.g., "Mon, Oct 27, 2025")
    char dateStr[30];
    strftime(dateStr, sizeof(dateStr), "%a, %b %d, %Y", &targetDate);
    return String(dateStr);
}

bool NetworkManager::fetchMockData(GoalData& data) {
    Serial.println("Using MOCK data for testing...");

    // Mock JSON response matching actual API format
    const char* mockJson = R"({
        "goal_tracking": {
            "current_progress_percent": 22.2
        },
        "projection": {
            "days_to_target": 3750
        },
        "metadata": {
            "data_timestamp": "2025-10-28T11:51:53.666Z"
        }
    })";

    // Parse mock JSON (ArduinoJson v7 uses dynamic allocation automatically)
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, mockJson);

    if (error) {
        Serial.print("Mock JSON parsing failed: ");
        Serial.println(error.c_str());
        return false;
    }

    // Extract data
    data.daysToGoal = doc["projection"]["days_to_target"];
    data.progressPercent = doc["goal_tracking"]["current_progress_percent"];

    // Extract and format timestamp from metadata
    const char* timestamp = doc["metadata"]["data_timestamp"];
    data.lastUpdateTime = formatTimestamp(timestamp);

    data.isValid = true;

    Serial.print("Days to goal: ");
    Serial.println(data.daysToGoal);
    Serial.print("Progress: ");
    Serial.print(data.progressPercent);
    Serial.println("%");
    Serial.print("Data timestamp: ");
    Serial.println(data.lastUpdateTime);

    return true;
}

bool NetworkManager::fetchGoalData(GoalData& data) {
    // Use mock data if enabled
    if (MOCK_MODE) {
        return fetchMockData(data);
    }

    // Real API call
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return false;
    }

    HTTPClient http;
    http.begin(API_URL);
    http.addHeader("Authorization", String("Bearer ") + API_TOKEN);
    http.setTimeout(10000);  // 10 second timeout

    Serial.println("Fetching data from API...");
    int httpCode = http.GET();

    if (httpCode == 200) {
        String payload = http.getString();
        Serial.println("Response received");

        // Parse JSON (ArduinoJson v7 uses dynamic allocation automatically)
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());
            http.end();
            return false;
        }

        // Extract data
        data.daysToGoal = doc["projection"]["days_to_target"];
        data.progressPercent = doc["goal_tracking"]["current_progress_percent"];

        // Extract and format timestamp from metadata
        const char* timestamp = doc["metadata"]["data_timestamp"];
        data.lastUpdateTime = formatTimestamp(timestamp);

        data.isValid = true;

        Serial.print("Days to goal: ");
        Serial.println(data.daysToGoal);
        Serial.print("Progress: ");
        Serial.print(data.progressPercent);
        Serial.println("%");
        Serial.print("Data timestamp: ");
        Serial.println(data.lastUpdateTime);

        http.end();
        return true;
    } else {
        Serial.print("HTTP error: ");
        Serial.println(httpCode);
        http.end();
        return false;
    }
}
