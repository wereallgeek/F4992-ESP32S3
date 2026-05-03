//================================================
// CPU Info helper
//
// handles various CPU related information
//
//================================================

//Internal temperature sensor============================================================================================
#include "driver/temperature_sensor.h"
temperature_sensor_handle_t temp_sensor = NULL;

void cpuInfo_setup() {
  cpuTempSensorSetup();
}

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
