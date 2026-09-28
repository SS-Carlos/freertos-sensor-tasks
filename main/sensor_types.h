#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "bmp280.h"
#include "mpu6050.h"
#include "sht30.h"

// Identify which sensor sends the data
typedef enum {
  SENSOR_BMP280,
  SENSOR_MPU6050,
  SENSOR_SHT30
} sensor_id_t;

// mensage which travel trought queue
typedef struct {
  sensor_id_t id;
  union {
    bmp280_data_t  bmp;
    mpu6050_data_t mpu;
    sht30_data_t   sht;
  } data;
} sensor_message_t;

// shared resources between tasks
typedef struct {
  SemaphoreHandle_t i2c_mutex;
  QueueHandle_t     data_queue;
} task_params_t;
