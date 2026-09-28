#include "bmp280.h"
#include "sensor_types.h"
#include "bmp280_task.h"

void bmp280_task(void *pvParameters)
{
  task_params_t *params = (task_params_t *)pvParameters;
  sensor_message_t msg;
  msg.id = SENSOR_BMP280;

  while (1) {
    if (xSemaphoreTake(params->i2c_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      bmp280_read(&msg.data.bmp);
      xSemaphoreGive(params->i2c_mutex);
    }

    xQueueSendToBack(params->data_queue, &msg, 0);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

}
