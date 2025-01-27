#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <NimBLEDevice.h>

#include "secrets.hpp"

const static uint32_t LOOP_INTERVAL_MS = 1 * 1000;

struct tm timeinfo;

static int32_t display_width = 0;
static int32_t display_height = 0;
static int32_t display_padding_x = 0;
static int32_t display_padding_y = 5;
const static uint8_t DISPLAY_BRIGHTNESS = 16;

const static uint8_t LIGHT_UNIT_DIGITAL_PIN = 32;
const static uint8_t LIGHT_UNIT_ANALOG_PIN = 33;
const static uint16_t LIGHT_SENSOR_THRESHOLD = 3000;

const static int32_t BATTERY_BAR_HEIGHT = 10;
static int32_t battery_level = 0;

const static char *BOT_SERVICE_UUID = "cba20d00-224d-11e6-9fb8-0002a5d5c51b";
const static char *BOT_CHARACTERISTIC_UUID = "cba20002-224d-11e6-9fb8-0002a5d5c51b";
const static uint8_t BOT_COMMAND[3] = {0x57, 0x01, 0x00};

void log(esp_log_level_t level, const char *format, ...)
{
  char timeStr[9] = {'0', '0', ':', '0', '0', ':', '0', '0', '\0'};
  if (getLocalTime(&timeinfo))
  {
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  }

  char message[256];
  va_list args;
  va_start(args, format);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);

  // for Serial
  M5.Log(level, "%s %s", timeStr, message);

  // for Display
  int32_t text_height = M5.Display.fontHeight();
  if (display_height <= M5.Display.getCursorY() + text_height * 3)
  {
    M5.Display.fillRect(0, 0, display_width, display_height - BATTERY_BAR_HEIGHT, BLACK);
    M5.Display.setCursor(display_padding_x, display_padding_y);
  }
  switch (level)
  {
  case ESP_LOG_VERBOSE:
    M5.Display.setTextColor(WHITE);
    break;
  case ESP_LOG_DEBUG:
    M5.Display.setTextColor(CYAN);
    break;
  case ESP_LOG_INFO:
    M5.Display.setTextColor(GREEN);
    break;
  case ESP_LOG_WARN:
    M5.Display.setTextColor(YELLOW);
    break;
  case ESP_LOG_ERROR:
    M5.Display.setTextColor(RED);
    break;
  default:
    M5.Display.setTextColor(WHITE);
    break;
  }
  M5.Display.printf("%s %s\n", timeStr, message);
}

void updateBatteryLevelDisplay()
{
  if (2 < abs(M5.Power.getBatteryLevel() - battery_level))
  {
    battery_level = M5.Power.getBatteryLevel();
    log(ESP_LOG_VERBOSE, "Battery: %d%%", battery_level);
    int bar_color = 0;
    switch (battery_level)
    {
    case 0 ... 20:
      bar_color = RED;
      break;
    case 21 ... 99:
      bar_color = ORANGE;
      break;
    case 100:
      bar_color = GREEN;
      break;
    }
    M5.Display.fillRect(0, display_height - BATTERY_BAR_HEIGHT, display_width, BATTERY_BAR_HEIGHT, BLACK);
    M5.Display.fillRect(0, display_height - BATTERY_BAR_HEIGHT, display_width * battery_level / 100, BATTERY_BAR_HEIGHT, bar_color);
  }
}

void syncWithNTP()
{
  configTime(9 * 3600, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  if (getLocalTime(&timeinfo))
  {
    log(ESP_LOG_INFO, "Time synchronized with NTP");
  }
  else
  {
    log(ESP_LOG_ERROR, "Failed to synchronize time");
  }
}

void executeBot()
{
  NimBLEDevice::init("elc");
  NimBLEClient *pClient = NimBLEDevice::createClient();
  if (pClient->connect(NimBLEAddress(SWITCHBOT_BLE_MAC, BLE_ADDR_RANDOM)))
  {
    log(ESP_LOG_INFO, "Connected to Bot");
    NimBLERemoteService *pRemoteService = pClient->getService(BOT_SERVICE_UUID);
    if (pRemoteService)
    {
      NimBLERemoteCharacteristic *pRemoteCharacteristic = pRemoteService->getCharacteristic(BOT_CHARACTERISTIC_UUID);
      if (pRemoteCharacteristic)
      {
        pRemoteCharacteristic->writeValue(BOT_COMMAND, sizeof(BOT_COMMAND), false);
        log(ESP_LOG_INFO, "Command Sent to Bot!");
      }
    }
    else
    {
      log(ESP_LOG_ERROR, "Failed to execute Bot");
    }
    NimBLEDevice::deinit(true);
  }
}

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Log.setEnableColor(m5::log_target_serial, false);
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_VERBOSE);
  M5.Display.setCursor(display_padding_x, display_padding_y);

  log(ESP_LOG_INFO, "Initialize");

  display_width = M5.Display.width();
  display_height = M5.Display.height();
  M5.Display.setTextWrap(true, true);
  M5.Display.setBrightness(DISPLAY_BRIGHTNESS);

  log(ESP_LOG_INFO, "Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    M5.delay(2 * 1000);
  }
  log(ESP_LOG_INFO, "IP Address: %s", WiFi.localIP().toString().c_str());
  syncWithNTP();
  WiFi.disconnect(true);

  setCpuFrequencyMhz(80);
}

void loop()
{
  M5.update();

  uint16_t light_value = analogRead(LIGHT_UNIT_ANALOG_PIN);
  if (light_value < 4095)
  {
    log(ESP_LOG_VERBOSE, "Light: %d", light_value);
  }
  if (light_value < LIGHT_SENSOR_THRESHOLD)
  {
    log(ESP_LOG_INFO, "Light is detected");
    executeBot();
  }

  if (M5.BtnA.wasPressed())
  {
    log(ESP_LOG_INFO, "Button A was pressed");
    executeBot();
  }

  updateBatteryLevelDisplay();

  M5.delay(LOOP_INTERVAL_MS);
}
