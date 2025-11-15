#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include "GoalData.h"
#include "config.h"

class DisplayManager {
public:
    static void init();
    static void showGoalInfo(const GoalData& data);
    static void showError(const char* message);
    static void hibernate();

private:
    static void drawErrorIcon();
    static GxEPD2_BW<GxEPD2_370_GDEY037T03, GxEPD2_370_GDEY037T03::HEIGHT>& getDisplay();
};

#endif // DISPLAY_MANAGER_H
