#ifndef GOAL_DATA_H
#define GOAL_DATA_H

#include <Arduino.h>

// Data structure for goal tracking information
struct GoalData {
    int daysToGoal;
    float progressPercent;
    String lastUpdateTime;
    String targetDate;
    bool isValid;
    bool lastUpdateSuccess;
};

// RTC memory storage (persists across deep sleep)
class DataStorage {
public:
    static void save(const GoalData& data);
    static bool load(GoalData& data);
    static bool hasData();

private:
    // RTC memory variables
    static RTC_DATA_ATTR int rtc_daysToGoal;
    static RTC_DATA_ATTR float rtc_progressPercent;
    static RTC_DATA_ATTR char rtc_lastUpdateTime[20];
    static RTC_DATA_ATTR char rtc_targetDate[30];
    static RTC_DATA_ATTR bool rtc_lastUpdateSuccess;
    static RTC_DATA_ATTR bool rtc_hasData;
};

#endif // GOAL_DATA_H
