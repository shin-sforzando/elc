#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <NimBLEDevice.h>

#include "secrets.hpp"

const static uint32_t LOOP_INTERVAL_MS = 1 * 1000;
const static int CPU_FREQ_MHZ = 80;

struct tm timeinfo;
const int TIMEZONE_OFFSET = 9 * 3600;
const int DAYLIGHT_OFFSET = 0;
const char *NTP_SERVER_1 = "ntp.nict.jp";
const char *NTP_SERVER_2 = "ntp.jst.mfeed.ad.jp";

static int32_t display_width = 0;
static int32_t display_height = 0;
static int32_t display_padding_x = 0;
static int32_t display_padding_y = 5;
const static uint8_t DISPLAY_BRIGHTNESS = 16;

const static uint8_t LIGHT_UNIT_DIGITAL_PIN = 32;
const static uint8_t LIGHT_UNIT_ANALOG_PIN = 33;
const static uint16_t LIGHT_SENSOR_THRESHOLD = 3000;
const static uint16_t DARK_THRESHOLD = 4000;

const static uint32_t BATTERY_CHECK_INTERVAL_MS = 10 * 1000;
const static int32_t BATTERY_BAR_HEIGHT = 10;
static int32_t battery_level = 0;
static unsigned long last_battery_checked = 0;

const static char *BOT_SERVICE_UUID = "cba20d00-224d-11e6-9fb8-0002a5d5c51b";
const static char *BOT_CHARACTERISTIC_UUID = "cba20002-224d-11e6-9fb8-0002a5d5c51b";
const static uint8_t BOT_COMMAND[3] = {0x57, 0x01, 0x00};

/**
 * @brief Log with timestamp.
 */
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

/**
 * @brief Change CPU frequency.
 */
void changeCpuFrequency(int frequency_mhz)
{
  Serial.flush();
  Serial.end();
  setCpuFrequencyMhz(frequency_mhz);
  Serial.begin(115200);
  log(ESP_LOG_INFO, "CPU: %d MHz", getCpuFrequencyMhz());
}

/**
 * @brief Check battery level and display battery bar.
 */
void checkBatteryLevel()
{
  int current_battery_level = M5.Power.getBatteryLevel();
  if (2 < abs(current_battery_level - battery_level))
  {
    battery_level = current_battery_level;
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

/**
 * @brief Synchronize time with NTP server.
 */
void syncWithNTP()
{
  configTime(TIMEZONE_OFFSET, DAYLIGHT_OFFSET, NTP_SERVER_1, NTP_SERVER_2);
  getLocalTime(&timeinfo) ? log(ESP_LOG_INFO, "Time synchronized with NTP") : log(ESP_LOG_ERROR, "Failed to synchronize time");
}

/**
 * @brief Read light sensor.
 */
bool readLightSensor()
{
  uint16_t light_value = analogRead(LIGHT_UNIT_ANALOG_PIN);
  if (light_value < DARK_THRESHOLD)
  {
    // If it's pitch dark.
    log(ESP_LOG_VERBOSE, "Light: %d", light_value);
  }
  if (light_value < LIGHT_SENSOR_THRESHOLD)
  {
    return true;
  }
  return false;
}

/**
 * @brief Execute Bot using BLE.
 */
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

/**
 * @brief Setup.
 */
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

  changeCpuFrequency(CPU_FREQ_MHZ);
  log(ESP_LOG_INFO, "Setup completed");
}

/**
 * @brief Loop.
 */
void loop()
{
  M5.update();

  if (readLightSensor())
  {
    log(ESP_LOG_INFO, "Light is detected");
    executeBot();
  }

  if (M5.BtnA.wasPressed())
  {
    log(ESP_LOG_INFO, "Button A was pressed");
    executeBot();
  }

  if (BATTERY_CHECK_INTERVAL_MS < millis() - last_battery_checked)
  {
    last_battery_checked = millis();
    checkBatteryLevel();
  }

  M5.delay(LOOP_INTERVAL_MS);
}
