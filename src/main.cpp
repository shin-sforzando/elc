#include <Arduino.h>
#include <M5Unified.h>

#include "secrets.hpp"

static int32_t display_width = 0;
static int32_t display_height = 0;
struct tm timeinfo;

void log(esp_log_level_t level, const char *format, ...)
{
  if (M5.Log.getLogLevel(m5::log_target_serial) == ESP_LOG_NONE)
  {
    M5.Log.setEnableColor(m5::log_target_serial, false);
    M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_VERBOSE);
  }

  char message[256];
  va_list args;
  va_start(args, format);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);

  // for Serial
  M5.Log(level, message);

  // for Display
  if (display_height <= M5.Display.getCursorY())
  {
    M5.Display.setCursor(0, 0);
  }
  M5.Display.println(message);
}

void configTimeFromNTP()
{
  configTime(9 * 3600, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
}

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);

  log(ESP_LOG_INFO, "Setting up ...");

  display_width = M5.Display.width();
  display_height = M5.Display.height();
}

void loop()
{
  M5.update();

  int32_t battery_level = M5.Power.getBatteryLevel();
  log(ESP_LOG_INFO, "Battery level: %d%%", battery_level);

  M5.delay(1000);
}
