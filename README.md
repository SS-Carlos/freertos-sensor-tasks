# FreeRTOS Sensor Tasks for ESP32

Multitask architecture for concurrent I2C sensor reading using FreeRTOS,
built on top of the bare-metal drivers from
[i2c-driver-bare-metal-esp32](https://github.com/SS-Carlos/i2c-driver-bare-metal-esp32).

Each sensor runs in its own independent FreeRTOS task, communicating
through a shared queue protected by a mutex.

## Architecture

```
┌─────────────────┐   ┌──────────────────┐   ┌─────────────────┐
│   bmp280_task   │   │   mpu6050_task   │   │   sht30_task    │
│   every 1000ms  │   │   every 100ms    │   │   every 2000ms  │
│                 │   │                  │   │                 │
│  mutex → read   │   │  mutex → read    │   │  mutex → read   │
│  give  → queue  │   │  give  → queue   │   │  give  → queue  │
└────────┬────────┘   └────────┬─────────┘   └────────┬────────┘
         │                     │                       │
         └─────────────────────┴───────────────────────┘
                               │
                               ▼ sensor_message_t
                    ┌─────────────────────┐
                    │    monitor_task     │
                    │                    │
                    │  xQueueReceive()   │
                    │  switch(sensor.id) │
                    │  ESP_LOGI(...)     │
                    └────────────────────┘
```

## FreeRTOS Resources

| Resource | Type | Purpose |
|----------|------|---------|
| `i2c_mutex` | Mutex | Protects shared I2C bus from concurrent access |
| `data_queue` | Queue (depth 10) | Transports sensor data to monitor task |

## Task Configuration

| Task | Priority | Stack | Period |
|------|----------|-------|--------|
| bmp280_task | 5 | 4096 B | 1000ms |
| mpu6050_task | 5 | 4096 B | 100ms |
| sht30_task | 5 | 4096 B | 2000ms |
| monitor_task | 3 | 4096 B | event-driven |

## Shared Data Structure

```c
// sensor_types.h — shared between all tasks
typedef enum {
    SENSOR_BMP280,
    SENSOR_MPU6050,
    SENSOR_SHT30
} sensor_id_t;

typedef struct {
    sensor_id_t id;
    union {
        bmp280_data_t  bmp;
        mpu6050_data_t mpu;
        sht30_data_t   sht;
    } data;
} sensor_message_t;

typedef struct {
    SemaphoreHandle_t i2c_mutex;
    QueueHandle_t     data_queue;
} task_params_t;
```

## File Structure

```
freertos-sensor-tasks/
├── components/
│   ├── bmp280/         — temperature and pressure driver
│   ├── mpu6050/        — accelerometer and gyroscope driver
│   └── sht30/          — temperature and humidity driver
└── main/
    ├── main.c          — bus init, sensors init, resources, task launch
    ├── sensor_types.h  — shared structs between all tasks
    ├── bmp280_task.c/h — BMP280 producer task
    ├── mpu6050_task.c/h — MPU6050 producer task
    ├── sht30_task.c/h  — SHT30 producer task
    └── monitor_task.c/h — data consumer task
```

## Hardware

- ESP32-D0WDQ6 (revision v1.0, dual core 240MHz)
- BMP280 — I2C address 0x76
- MPU6050 GY-521 — I2C address 0x68
- SHT30 — I2C address 0x44
- SDA → GPIO 21 / SCL → GPIO 22

## Known Limitation

`sht30_task` holds the I2C mutex during the 30ms internal measurement
delay inside `sht30_read()`. A future improvement will split the
measurement into `sht30_start_measurement()` and `sht30_read_data()`
to release the mutex during the measurement window.

This will be addressed in the next repository with an I2C manager
task pattern — a dedicated task that exclusively owns the bus and
serves requests from other tasks via queue.

## Validated

System tested continuously for 2 hours without errors or watchdog
triggers. All three sensors reading concurrently without I2C bus
collisions.

## References

- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [ESP-IDF FreeRTOS API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos.html)
- [i2c-driver-bare-metal-esp32](https://github.com/SS-Carlos/i2c-driver-bare-metal-esp32)

## License

MIT © Carlos Solano 2026
