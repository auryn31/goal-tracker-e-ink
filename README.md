# Goal Tracker - E-ink Display

[YouTube Video](https://youtu.be/ImAW_xP-6EQ)

ESP32-based goal progress tracker that displays days remaining until your target date and goal progress on a 3.7" e-ink display (240x416). Perfect for tracking retirement savings, financial goals, project deadlines, or any long-term objective.

**⚠️ IMPORTANT: Before using this project, copy `include/config.example.h` to `include/config.h` and add your WiFi credentials and API token. Never commit `config.h` to version control!**

## Features

- Real-time goal countdown (days remaining until target)
- Progress percentage tracking
- Visual progress bar
- Automatic hourly updates via WiFi
- **Ultra-low power consumption** - Deep sleep between updates (~150µA)
- E-ink display retains image without power
- Configurable for any goal type (retirement, savings, projects, etc.)
- Secure configuration (WiFi and API credentials excluded from version control)

## Hardware Requirements

- **ESP32-C3 Super Mini** board
- WeAct Studio 3.7" e-ink display (240x416 resolution)
- USB-C cable for programming

## Display Wiring

Connect your WeAct Studio e-ink display to ESP32-C3 Super Mini:

| Display Pin | ESP32-C3 Pin | Notes |
|-------------|--------------|-------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |
| SDA (MOSI) | GPIO6 | SPI data to display |
| SCL (SCK) | GPIO4 | SPI clock |
| CS | GPIO7 | Chip Select |
| D/C | GPIO1 | Data/Command |
| RES (RST) | GPIO2 | Reset |
| BUSY | GPIO3 | Busy signal |

**ESP32-C3 Super Mini + WeAct Display Notes:**
- **CRITICAL**: GPIO8 must be HIGH to power the display (handled in code)
- Pin mapping based on WeAct Studio's official example code
- **MOSI=GPIO6**, **SCK=GPIO4** (different from standard ESP32-C3 SPI!)
- Display model: GDEY037T03 (240x416, UC8253 controller)
- USB uses GPIO18/GPIO19 (reserved for programming)

## ⚠️ Troubleshooting "Busy Timeout" Errors

If you see `Busy Timeout!` in serial monitor:

### 1. Verify Wiring (Most Common Issue)
Use a multimeter or visual inspection:

```
Display Pin  →  ESP32-C3     Check
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
VCC          →  3.3V         ✓ Red wire, proper connection
GND          →  GND          ✓ Black wire, proper ground
SDA          →  GPIO6        ✓ Data line
SCL          →  GPIO4        ✓ Clock line
CS           →  GPIO7        ✓ Chip select
D/C          →  GPIO1        ✓ Data/Command
RST          →  GPIO2        ✓ Reset
BUSY         →  GPIO3        ✓ THIS IS CRITICAL - must be connected!
```

**BUSY pin is the most important** - if it's not connected or loose, you'll get timeout errors.

### 2. Check Power
- Measure voltage at display VCC pin: should be ~3.3V
- GPIO8 should be HIGH (you can measure it)
- Try a different USB cable (some provide poor power)

### 3. Serial Monitor Output
After uploading, check for:
```
Display power enabled!
BUSY pin state: LOW        ← Should show LOW or HIGH
BUSY pin after init: ...   ← Should change state
```

If BUSY pin is always the same state, the display isn't responding.

### 4. Physical Display Check
- Look for a model number on the back: GDEY037T03, GDEW037T03, etc.
- If different model, update line 16 in main.cpp
- Check for any obvious damage to the display ribbon cable

## Setup Instructions

### 1. Clone and Configure

```bash
# Copy the example config file
cp include/config.example.h include/config.h
```

### 2. Edit Configuration

Edit `include/config.h` with your settings:

```cpp
// WiFi credentials
const char* WIFI_SSID = "YourWiFiName";
const char* WIFI_PASSWORD = "YourWiFiPassword";

// API endpoint - replace with your server IP
const char* API_URL = "http://192.168.1.100:4000/api/portfolio/summary";
const char* API_TOKEN = "your_api_token_here";

// Mock mode for testing (set to true to test without WiFi/API)
const bool MOCK_MODE = false;  // Set to true for testing

// Display pins (ESP32-C3 Super Mini)
#define EPD_CS 7      // D7
#define EPD_DC 6      // D6
#define EPD_RST 3     // D3
#define EPD_BUSY 2    // D2
#define EPD_MOSI 4    // D4 - Connect to display SDA
#define EPD_SCK 5     // D5 - Connect to display SCL
```

### 3. Testing with Mock Data (Optional)

For initial testing without WiFi or API server:

1. Set `MOCK_MODE = true` in `include/config.h`
2. This will use sample data from your actual API response
3. Perfect for testing display layout and deep sleep functionality
4. Set back to `false` when ready to use real API

### 4. Build and Upload

```bash
# Install dependencies and build
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

**For faster testing:** Set `UPDATE_INTERVAL = 300000` (5 minutes) in config.h to see multiple wake cycles quickly.

## Display Layout

The screen shows:

```
┌─────────────────────┐
│      GOAL           │
│    TRACKER          │
├─────────────────────┤
│                     │
│    DAYS LEFT        │
│                     │
│       3561          │
│                     │
├─────────────────────┤
│                     │
│    PROGRESS         │
│                     │
│      34.9%          │
│                     │
│  [████░░░░░░░░░]    │
│                     │
│ ~ 9 years, 8 months │
│                     │
│ Updates hourly      │
└─────────────────────┘
```

## How It Works

1. **ESP32 wakes up** from deep sleep (or powers on)
2. **Connects to WiFi** and fetches data from your API
3. **Updates e-ink display** with current goal progress data
4. **Powers down display** to hibernate mode
5. **Enters deep sleep** for 1 hour (configurable)
6. **Repeats** - ESP32 wakes up automatically after sleep timer

**Power Consumption:**
- Active (fetching & updating): ~80-240mA for ~10-30 seconds
- Deep sleep: ~150µA (0.15mA)
- Average over 1 hour: < 1mA
- E-ink display: 0mA (retains image without power)

## API Response Format

The code expects JSON from your API endpoint:

```json
{
  "goal_tracking": {
    "current_progress_percent": 34.87
  },
  "projection": {
    "days_to_target": 3561
  }
}
```

## Configuration Options

### Mock Mode (Testing)

Test the display without WiFi or API server:

```cpp
const bool MOCK_MODE = true;  // Uses sample data
```

When enabled:
- Skips WiFi connection
- Uses hardcoded sample data (3561 days, 34.87%)
- Perfect for testing display and sleep functionality
- Useful for initial hardware testing

### Update Interval

Adjust in `include/config.h`:
```cpp
const unsigned long UPDATE_INTERVAL = 3600000; // milliseconds
```

Common values:
- 1 hour: `3600000` (recommended for production)
- 30 minutes: `1800000`
- 5 minutes: `300000` (testing only)

### Display Rotation

```cpp
#define DISPLAY_ROTATION 0  // 0=portrait, 1=landscape, 2=inverted portrait, 3=inverted landscape
```

### Display Model

If your display doesn't work, check the model number on the back and update in `src/main.cpp`:

```cpp
// Try different models:
GxEPD2_BW<GxEPD2_371, GxEPD2_371::HEIGHT> display(GxEPD2_371(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
// or
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
```

## Battery Operation

With deep sleep, this project can run on battery for extended periods:

**Estimated battery life (with 3.7V LiPo battery):**
- 1000mAh: ~40 days
- 2000mAh: ~80 days
- 3000mAh: ~120 days

*Actual battery life depends on WiFi signal strength, API response time, and battery quality.*

**Battery Connection (ESP32-C3 Super Mini):**
- Connect 3.7V LiPo battery to BAT+ and GND pads on bottom of board
- Built-in charging via USB-C when battery is connected
- No external charging module needed
- Charging LED indicates charging status

## Troubleshooting

### Display not updating
1. Check wiring connections
2. Verify display model in code matches your hardware
3. Check serial monitor for error messages during wake cycle

### WiFi connection failed
1. Verify SSID and password in `config.h`
2. Ensure ESP32 is within WiFi range
3. Check serial monitor for connection status
4. Weak WiFi signal increases power consumption

### API errors
1. Verify API URL is correct (use local IP, not localhost)
2. Check that server is accessible from ESP32's network
3. Verify API token is valid
4. Check serial monitor for HTTP error codes

### Device keeps rebooting
1. Power supply may be insufficient - use good quality USB cable or battery
2. Check for brownout detector resets in serial monitor

### Compilation errors
1. Run `pio lib install` to ensure all dependencies are installed
2. Check that `include/config.h` exists (copy from `config.example.h`)

### Can't see serial output after first boot
- Normal behavior! Serial disconnects during deep sleep
- To debug: Temporarily increase `UPDATE_INTERVAL` to a few minutes
- Or: Press reset button to see a fresh boot cycle

### ESP32-C3 Specific Issues

**Board not detected:**
- ESP32-C3 Super Mini uses native USB (not USB-to-Serial chip)
- May need to hold BOOT button while plugging in USB-C
- Drivers: Usually works automatically on macOS/Linux
- Windows: May need ESP32-C3 USB CDC driver

**Boot loop or won't start:**
- Check that GPIO8, GPIO9 are not pulled low during boot
- These are strapping pins on ESP32-C3
- Ensure display connections are correct

**Upload fails:**
1. Hold BOOT button
2. Press RESET button (while holding BOOT)
3. Release RESET, then release BOOT
4. Try uploading again

## Security

**🔒 CRITICAL - Protecting Your Credentials:**

1. **`include/config.h` contains sensitive data** and is excluded from version control
2. **NEVER commit your actual `config.h`** - it contains:
   - WiFi SSID and password
   - API authentication token
   - Local network IP addresses
3. **Always use `config.example.h`** as the template to share
4. **Before publishing/sharing:**
   - Run `git status` to verify `config.h` is not tracked
   - Delete `config.h` from your working directory if sharing the folder
   - Recreate it locally from `config.example.h` after cloning

**If you accidentally committed `config.h`:**
```bash
# Remove from git history (WARNING: rewrites history)
git filter-branch --force --index-filter \
  'git rm --cached --ignore-unmatch include/config.h' \
  --prune-empty --tag-name-filter cat -- --all

# Then force push (if already pushed to remote)
git push origin --force --all
```

**After removing from git history, you MUST:**
- Change your WiFi password
- Regenerate your API token
- Update your local `config.h` with new credentials

## Dependencies

- ArduinoJson (^7.2.0)
- GxEPD2 (^1.6.0)
- Adafruit GFX Library (^1.11.11)

## License

This project is open source. Adjust as needed for your use case.
