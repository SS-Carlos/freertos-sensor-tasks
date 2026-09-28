#include "monitor_task.h"
#include "esp_log.h"

static const char *TAG = "MONITOR";

void monitor_task(void *pvParameters)
{
  task_params_t   *params = (task_params_t *)pvParameters;
  sensor_message_t received;

  while (1) {
    if (xQueueReceive(params->data_queue, &received, portMAX_DELAY) == pdTRUE) {
      switch (received.id) {
        case SENSOR_BMP280:
          ESP_LOGI(TAG, "BMP280 - Temp: %.2f C Press: %.2f hPa", 
                   received.data.bmp.temperature,
                   received.data.bmp.pressure);
        break;
        case SENSOR_MPU6050:
          ESP_LOGI(TAG, "MPU6050 - Accel: X=%.2f Y=%.2f Z=%.2f g",
                   received.data.mpu.accel_x, 
                   received.data.mpu.accel_y, 
                   received.data.mpu.accel_z);
          ESP_LOGI(TAG, "MPU6050 - Gyro:  X=%.2f Y=%.2f Z=%.2f °/s",
                   received.data.mpu.gyro_x, 
                   received.data.mpu.gyro_y, 
                   received.data.mpu.gyro_z);
        break;
        case SENSOR_SHT30:
          ESP_LOGI(TAG, "SHT30 - Temp: %.2f C Humidity: %.2f %%",
                   received.data.sht.temperature, 
                   received.data.sht.humidity);
        break;

        default:
        ESP_LOGW(TAG, "Unknown Sensor ID: %d", received.id);
        break;
      }
    }
  }
}
