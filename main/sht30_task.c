#include "sht30.h"
#include "sensor_types.h"
#include "sht30_task.h"

void sht30_task( void * pvParamters)
{
  task_params_t *params = (task_params_t *)pvParamters;
  sensor_message_t msg;
  msg.id = SENSOR_SHT30;

  while (1) {
    if (xSemaphoreTake(params->i2c_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      sht30_read(&msg.data.sht);
      xSemaphoreGive(params->i2c_mutex);
    }

    xQueueSendToBack(params->data_queue, &msg, 0);

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

