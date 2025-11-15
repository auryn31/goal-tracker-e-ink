#include "DisplayManager.h"
#include <SPI.h>

// Layout constants for display positioning
#define MARGIN_LEFT 10
#define MARGIN_RIGHT 20
#define MARGIN_TOP 10
#define MARGIN_BOTTOM 10
#define TITLE_Y 25
#define MAIN_TEXT_Y 80
#define SUBTITLE_Y 105
#define DETAIL_Y 120
#define DIVIDER_X 200
#define DIVIDER_END_Y 140
#define PROGRESS_BAR_Y 150
#define PROGRESS_BAR_HEIGHT 25
#define DATE_Y 196
#define ERROR_ICON_SIZE 12
#define ERROR_ICON_MARGIN 25

// Initialize display instance
static GxEPD2_BW<GxEPD2_370_GDEY037T03, GxEPD2_370_GDEY037T03::HEIGHT> display(
    GxEPD2_370_GDEY037T03(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

GxEPD2_BW<GxEPD2_370_GDEY037T03, GxEPD2_370_GDEY037T03::HEIGHT>& DisplayManager::getDisplay() {
    return display;
}

void DisplayManager::init() {
    Serial.println("Initializing display...");

    // Enable display power (GPIO8 must be HIGH)
    pinMode(EPD_POWER_PIN, OUTPUT);
    digitalWrite(EPD_POWER_PIN, HIGH);
    delay(500);  // Power stabilization

    // Initialize SPI with custom pins
    SPI.begin(EPD_SCK, -1, EPD_MOSI, EPD_CS);

    // Initialize display
    display.init(115200, true, 50, false);
    display.setRotation(DISPLAY_ROTATION);
    display.setTextColor(GxEPD_BLACK);

    Serial.print("Display initialized: ");
    Serial.print(display.width());
    Serial.print("x");
    Serial.println(display.height());
}

void DisplayManager::drawErrorIcon() {
    // Draw a small "X" icon in circle to indicate error (top right)
    int iconX = display.width() - ERROR_ICON_MARGIN;
    int iconY = MARGIN_TOP;
    display.drawLine(iconX, iconY, iconX + 10, iconY + 10, GxEPD_BLACK);
    display.drawLine(iconX + 10, iconY, iconX, iconY + 10, GxEPD_BLACK);
    display.drawCircle(iconX + 5, iconY + 5, ERROR_ICON_SIZE, GxEPD_BLACK);
}


void DisplayManager::showGoalInfo(const GoalData& data) {
    display.setFullWindow();
    display.firstPage();

    do {
        display.fillScreen(GxEPD_WHITE);

        // Title at top left
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(MARGIN_LEFT, TITLE_Y);
        display.print("DAYS TO GOAL");

        // Error/offline indicator - top right
        if (!data.lastUpdateSuccess) {
            drawErrorIcon();
        }

        // Left side - Days remaining
        display.setFont(&FreeMonoBold24pt7b);
        display.setCursor(MARGIN_LEFT, MAIN_TEXT_Y);
        display.print(data.daysToGoal);

        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(MARGIN_LEFT, SUBTITLE_Y);
        display.print("DAYS LEFT");

        // Calculate years from days
        int years = data.daysToGoal / 365;
        int months = (data.daysToGoal % 365) / 30;

        display.setFont();
        display.setCursor(MARGIN_LEFT, DETAIL_Y);
        display.print("~");
        display.print(years);
        display.print("y ");
        display.print(months);
        display.print("m");

        // Vertical divider line (stops before progress bar)
        display.drawLine(DIVIDER_X, MARGIN_TOP, DIVIDER_X, DIVIDER_END_Y, GxEPD_BLACK);

        // Right side - Progress percentage
        display.setFont(&FreeMonoBold24pt7b);
        display.setCursor(DIVIDER_X + 20, MAIN_TEXT_Y);
        display.print(data.progressPercent, 1);
        display.print("%");

        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(DIVIDER_X + 20, SUBTITLE_Y);
        display.print("COMPLETE");

        // Progress bar - horizontal at bottom
        int barX = MARGIN_LEFT;
        int barY = PROGRESS_BAR_Y;
        int barWidth = display.width() - MARGIN_LEFT - MARGIN_RIGHT;
        int barHeight = PROGRESS_BAR_HEIGHT;

        // Draw outline
        display.drawRect(barX, barY, barWidth, barHeight, GxEPD_BLACK);

        // Fill progress (with bounds checking)
        int fillWidth = (int)((data.progressPercent / 100.0) * (barWidth - 4));
        // Ensure fillWidth doesn't exceed bar bounds
        if (fillWidth > barWidth - 4) {
            fillWidth = barWidth - 4;
        }
        if (fillWidth > 0) {
            display.fillRect(barX + 2, barY + 2, fillWidth, barHeight - 4, GxEPD_BLACK);
        }

        // Target date display - centered below progress bar
        display.setFont(&FreeMonoBold12pt7b);
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(data.targetDate, 0, 0, &x1, &y1, &w, &h);
        int dateX = (display.width() - w) / 2;
        display.setCursor(dateX, DATE_Y);
        display.print(data.targetDate);

        // Bottom info
        display.setFont();

        // Left: Last update time
        display.setCursor(MARGIN_LEFT, display.height() - MARGIN_BOTTOM);
        if (data.lastUpdateSuccess) {
            display.print("Updated: ");
            display.print(data.lastUpdateTime);
        } else {
            display.print("Last: ");
            display.print(data.lastUpdateTime);
            display.print(" (offline)");
        }

    } while (display.nextPage());

    Serial.println("Display updated");
}

void DisplayManager::showError(const char* message) {
    display.setFullWindow();
    display.firstPage();

    do {
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(20, 100);
        display.print("ERROR:");
        display.setCursor(20, 140);
        display.print(message);
    } while (display.nextPage());

    Serial.print("Error displayed: ");
    Serial.println(message);
}

void DisplayManager::hibernate() {
    display.hibernate();

    // Turn off display power to save energy during deep sleep
    digitalWrite(EPD_POWER_PIN, LOW);
    pinMode(EPD_POWER_PIN, INPUT);  // Set to high impedance

    Serial.println("Display hibernated and powered off");
}
