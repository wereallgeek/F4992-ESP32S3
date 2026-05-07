//================================================
// CPU Info helper
//
// handles various CPU related information
//
//================================================

//Internal temperature sensor============================================================================================
#include <Arduino.h>
#include "driver/temperature_sensor.h"

RTC_DATA_ATTR int lastResetReason;
RTC_DATA_ATTR int lastCrashReason = 0;
temperature_sensor_handle_t temp_sensor = NULL;



void cpuInfo_setup() {
  lastResetReason = esp_reset_reason();
  if (lastResetReason == ESP_RST_PANIC || lastResetReason == ESP_RST_TASK_WDT || 
      lastResetReason == ESP_RST_WDT || lastResetReason == ESP_RST_INT_WDT) {
    lastCrashReason = lastResetReason;
  }
  cpuTempSensorSetup();
}


//firmware ==============================================================================================================
String firmwareVersion() {
  static String version = ""; 
  if (version != "") return version; //compute once print many

  static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  version = String(__DATE__ + 7); 
  int m = (String(months).indexOf(String(__DATE__).substring(0, 3)) / 3) + 1;
  if (m < 10) version += "0";
  version += m;
  version += (__DATE__[4] == ' ') ? "0" : String(__DATE__[4]);
  version += __DATE__[5];
  version += "-";
  version += String(__TIME__).substring(0, 2);
  version += String(__TIME__).substring(3, 5);

  return version;
}
//firmware ==============================================================================================================

//CPU related ===========================================================================================================
String getChipModel() {
  return String(ESP.getChipModel());
}

uint8_t getCpuCores() {
  return ESP.getChipCores();
}

String getCpuFreqMHz() {
  return String(ESP.getCpuFreqMHz()) + " MHz";
}
//CPU related ===========================================================================================================

//Internal temperature sensor============================================================================================
void cpuTempSensorSetup() {
  temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
  temperature_sensor_install(&temp_sensor_config, &temp_sensor);
  temperature_sensor_enable(temp_sensor);
}

float getCpuTemperature() {
  float tsens_out;
  temperature_sensor_get_celsius(temp_sensor, &tsens_out);
  return tsens_out;
}
//Internal temperature sensor============================================================================================


//reboot reason==========================================================================================================
int getResetReason() {
  return lastResetReason;
}

String getReadableResetReason() {
  switch (lastResetReason) {
    case ESP_RST_UNKNOWN:   return "Unknown";
    case ESP_RST_POWERON:   return "Power-on / Cold start";
    case ESP_RST_EXT:       return "External Reset button";
    case ESP_RST_SW:        return "Software Reset";
    case ESP_RST_PANIC:     return "Exception / Crash (Panic)";
    case ESP_RST_INT_WDT:   return "Watchdog (Interrupt)";
    case ESP_RST_TASK_WDT:  return "Watchdog (Task blocked)";
    case ESP_RST_WDT:       return "Other Watchdog";
    case ESP_RST_DEEPSLEEP: return "Deep Sleep Wakeup";
    case ESP_RST_BROWNOUT:  return "Brownout (Voltage drop)";
    case ESP_RST_SDIO:      return "Reset over SDIO";
    default:                return "Other (" + String(lastResetReason) + ")";
  }
}

String getReadableLastCrashReason() {
  switch (lastCrashReason) {
    case 0:                 return "None";
    case ESP_RST_PANIC:     return "Exception / Crash (Panic)";
    case ESP_RST_INT_WDT:   return "Watchdog (Interrupt)";
    case ESP_RST_TASK_WDT:  return "Watchdog (Task blocked)";
    case ESP_RST_WDT:       return "Other Watchdog";
    default:                return "Other (" + String(lastCrashReason) + ")";
  }
}
//reboot reason==========================================================================================================

//Memory-related=========================================================================================================
uint32_t getFreeHeap() {
  return ESP.getFreeHeap();
}

uint32_t getTotalHeap() {
  return ESP.getHeapSize();
}

uint32_t getFlashSize() {
  return ESP.getFlashChipSize();
}

uint32_t getSketchSize() {
  return ESP.getSketchSize();
}

uint32_t getFreeSketchSpace() {
  return ESP.getFreeSketchSpace();
}
//Memory-related=========================================================================================================
