#include "GoalData.h"

// Initialize static RTC memory variables
RTC_DATA_ATTR int DataStorage::rtc_daysToGoal = 0;
RTC_DATA_ATTR float DataStorage::rtc_progressPercent = 0.0;
RTC_DATA_ATTR char DataStorage::rtc_lastUpdateTime[20] = "";
RTC_DATA_ATTR char DataStorage::rtc_targetDate[30] = "";
RTC_DATA_ATTR bool DataStorage::rtc_lastUpdateSuccess = false;
RTC_DATA_ATTR bool DataStorage::rtc_hasData = false;

void DataStorage::save(const GoalData& data) {
    rtc_daysToGoal = data.daysToGoal;
    rtc_progressPercent = data.progressPercent;
    strncpy(rtc_lastUpdateTime, data.lastUpdateTime.c_str(), 19);
    rtc_lastUpdateTime[19] = '\0';
    strncpy(rtc_targetDate, data.targetDate.c_str(), 29);
    rtc_targetDate[29] = '\0';
    rtc_lastUpdateSuccess = data.lastUpdateSuccess;
    rtc_hasData = true;

    Serial.println("Data saved to RTC memory");
}

bool DataStorage::load(GoalData& data) {
    if (!rtc_hasData) {
        Serial.println("No cached data in RTC memory");
        return false;
    }

    data.daysToGoal = rtc_daysToGoal;
    data.progressPercent = rtc_progressPercent;
    data.lastUpdateTime = String(rtc_lastUpdateTime);
    data.targetDate = String(rtc_targetDate);
    data.lastUpdateSuccess = rtc_lastUpdateSuccess;
    data.isValid = true;

    Serial.println("Data loaded from RTC memory");
    return true;
}

bool DataStorage::hasData() {
    return rtc_hasData;
}
