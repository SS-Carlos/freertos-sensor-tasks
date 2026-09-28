#include "mpu6050.h"
#include "sensor_types.h"
#include "mpu6050_task.h"

void mpu6050_task(void *pvParameters)
{
  task_params_t *params = (task_params_t *)pvParameters;
  sensor_message_t msg;
  msg.id = SENSOR_MPU6050;

  while (1) {
    if (xSemaphoreTake(params->i2c_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      mpu6050_read(&msg.data.mpu);
      xSemaphoreGive(params->i2c_mutex);
    }

    xQueueSendToBack(params->data_queue, &msg, 0);

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
