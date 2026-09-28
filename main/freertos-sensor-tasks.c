#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_check.h"

#include "bmp280.h"
#include "mpu6050.h"
#include "sht30.h"

#include "sensor_types.h"
#include "bmp280_task.h"
#include "mpu6050_task.h"
#include "sht30_task.h"
#include "monitor_task.h"

static const char *TAG = "MAIN";

static QueueHandle_t     data_queue = NULL;
static SemaphoreHandle_t i2c_mutex  = NULL;
static task_params_t params;

void app_main(void)
{
  /*--- Initialize shared I2C bus -------*/
  i2c_master_bus_handle_t bus_handle;
  i2c_master_bus_config_t bus_cfg = {
      .i2c_port                     = I2C_NUM_0,
      .sda_io_num                   = GPIO_NUM_21,
      .scl_io_num                   = GPIO_NUM_22,
      .clk_source                   = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt            = 7,
      .flags.enable_internal_pullup = true,
  };
  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle));

  /*--- Initialize sensors -------------*/
  bmp280_config_t  bmp_config = BMP280_DEFAULT_CONFIG;
  mpu6050_config_t mpu_config = MPU6050_DEFAULT_CONFIG;
  sht30_config_t   sht_config = SHT30_DEFAULT_CONFIG;

  ESP_ERROR_CHECK(bmp280_init(bus_handle, &bmp_config));
  ESP_ERROR_CHECK(mpu6050_init(bus_handle, &mpu_config));
  ESP_ERROR_CHECK(sht30_init(bus_handle, &sht_config));

  /*--- Create RTOS resources ------*/
  
  data_queue = xQueueCreate(10, sizeof(sensor_message_t));
  i2c_mutex  = xSemaphoreCreateMutex();

  if (data_queue == NULL || i2c_mutex == NULL) {
    ESP_LOGE(TAG, "Failed to create FreeRTOS resources");
    return;
  }

  /*--- Create tasks -----------------*/
  params.i2c_mutex  = i2c_mutex;
  params.data_queue = data_queue;

  xTaskCreate(bmp280_task,  "bmp280",  4096, &params, 5, NULL);
  xTaskCreate(mpu6050_task, "mpu6050", 4096, &params, 5, NULL);
  xTaskCreate(sht30_task,   "sht30",   4096, &params, 5, NULL);
  xTaskCreate(monitor_task, "monitor", 4096, &params, 3, NULL);
}
