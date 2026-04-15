#include "stm32c5xx_hal.h"
#include "FreeRTOS.h"
#include "stm32c5xx_hal_gpio.h"
#include "stm32c5xx_hal_uart.h"
#include "task.h"
#include "semphr.h"
#include "mx_gpio_default.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef PROTECTION_MODE
#define PROTECTION_MODE 0
#endif

typedef struct {
    uint32_t raw_timestamp;
    uint32_t time_diff;
    float gyro_raw[3];
    float accel_raw[3];

    uint32_t task_b_timestamp;
    float gyro_avg[3];
    float gyro_power;
    float accel_avg[3];
    float accel_power;
    uint32_t task_b_end_time;
} sensor_data_t;

static sensor_data_t sensor_data;
static hal_uart_handle_t *uart_handle;

#if PROTECTION_MODE == 1
static SemaphoreHandle_t data_lock;
static SemaphoreHandle_t uart_lock;
#elif PROTECTION_MODE == 2
static SemaphoreHandle_t data_lock;
static SemaphoreHandle_t uart_lock;
#endif

#define SYNC_TIMEOUT (PROTECTION_MODE > 0 ? pdMS_TO_TICKS(5) : portMAX_DELAY)

hal_uart_handle_t *mx_usart2_uart_gethandle(void);
static void toggle_led(int led);
static float get_noise(float amp);
static void print_uart(const char *msg);

static bool lock_data(void) {
#if PROTECTION_MODE == 1
    return xSemaphoreTake(data_lock, SYNC_TIMEOUT) == pdTRUE;
#elif PROTECTION_MODE == 2
    return xSemaphoreTakeRecursive(data_lock, SYNC_TIMEOUT) == pdTRUE;
#else
    return true;
#endif
}

static void unlock_data(void) {
#if PROTECTION_MODE == 1
    xSemaphoreGive(data_lock);
#elif PROTECTION_MODE == 2
    xSemaphoreGiveRecursive(data_lock);
#endif
}

static void task_a_sensor(void *params) {
    TickType_t last_wake = xTaskGetTickCount();
    uint32_t prev_ts = 0;
    uint32_t last_uart_ts = 0;
    char buf[128];

    while (1) {
        toggle_led(0);
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));

        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        //OBLICZANIE DANYCH

        float gyro[3] = {0.50f + get_noise(0.10f), -0.30f + get_noise(0.10f), 0.20f + get_noise(0.10f)};
        float accel[3] = {0.05f + get_noise(0.02f), -0.04f + get_noise(0.02f), 9.89f + get_noise(0.02f)};

        // zaPIS DO STRUKTUR
        if (lock_data()) {
            sensor_data.raw_timestamp = now;
            sensor_data.time_diff = (prev_ts == 0) ? 0 : (now - prev_ts);
            memcpy(sensor_data.gyro_raw, gyro, sizeof(gyro));
            memcpy(sensor_data.accel_raw, accel, sizeof(accel));
            unlock_data();
        }
        prev_ts = now;

        if (now - last_uart_ts >= 1000) {
            last_uart_ts = now;
            snprintf(buf, sizeof(buf), "Nowe dane t=%lu ms\r\n", now);
            print_uart(buf);
        }
        toggle_led(0);
    }
}

// Task B
static void task_b_calc(void *params) {
    float gyro_avg[3] = {0}, accel_avg[3] = {0};
    uint32_t last_uart_ts = 0;

    while (1) {
        toggle_led(1);
        float gyro_raw[3], accel_raw[3];
        uint32_t raw_ts = 0;

        if (lock_data()) {
            raw_ts = sensor_data.raw_timestamp;
            memcpy(gyro_raw, sensor_data.gyro_raw, sizeof(gyro_raw));
            memcpy(accel_raw, sensor_data.accel_raw, sizeof(accel_raw));
            unlock_data();
        }

        const float alpha = 0.97f;
        for (int i = 0; i < 3; i++) {
            gyro_avg[i] = alpha * gyro_avg[i] + (1.0f - alpha) * gyro_raw[i];
            accel_avg[i] = alpha * accel_avg[i] + (1.0f - alpha) * accel_raw[i];
        }

        float g_power = (gyro_avg[0]*gyro_avg[0]) + (gyro_avg[1]*gyro_avg[1]) + (gyro_avg[2]*gyro_avg[2]);
        float a_power = (accel_avg[0]*accel_avg[0]) + (accel_avg[1]*accel_avg[1]) + (accel_avg[2]*accel_avg[2]);
        uint32_t end_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

        if (lock_data()) {
            sensor_data.task_b_timestamp = raw_ts;
            memcpy(sensor_data.gyro_avg, gyro_avg, sizeof(gyro_avg));
            memcpy(sensor_data.accel_avg, accel_avg, sizeof(accel_avg));
            sensor_data.gyro_power = g_power;
            sensor_data.accel_power = a_power;
            sensor_data.task_b_end_time = end_time;
            unlock_data();
        }

        if (end_time - last_uart_ts >= 1000) {
            last_uart_ts = end_time;
            print_uart("Obliczenia zrobione\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

static void task_c_monitor(void *params) {
    TickType_t last_wake = xTaskGetTickCount();
    char buf[128];

    while (1) {
        toggle_led(3);
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(71));

        sensor_data_t snap;
        if (!lock_data()) continue;
        snap = sensor_data;
        unlock_data();

//bład
        if (snap.raw_timestamp != snap.task_b_timestamp) {
            snprintf(buf, sizeof(buf), "ALARM %lu != %lu\r\n",
                     snap.raw_timestamp, snap.task_b_timestamp);
            print_uart(buf);
        }

        // Proste sprawdzanie opóźnień rtos
        if (snap.time_diff != 0 && snap.time_diff != 10) {
            snprintf(buf, sizeof(buf), "ALARM %lu ms\r\n", snap.time_diff);
            print_uart(buf);
        }
        toggle_led(3);
    }
}

int32_t app_synctasks_init(void) {
    uart_handle = mx_usart2_uart_gethandle();
    memset(&sensor_data, 0, sizeof(sensor_data));

#if PROTECTION_MODE == 1
    data_lock = xSemaphoreCreateBinary();
    xSemaphoreGive(data_lock);
    uart_lock = xSemaphoreCreateBinary();
    xSemaphoreGive(uart_lock);
#elif PROTECTION_MODE == 2
    data_lock = xSemaphoreCreateRecursiveMutex();
    uart_lock = xSemaphoreCreateRecursiveMutex();
#endif

    xTaskCreate(task_a_sensor, "TaskA", 512, NULL, 3, NULL);
    xTaskCreate(task_b_calc, "TaskB", 512, NULL, 2, NULL);
    xTaskCreate(task_c_monitor, "TaskC", 512, NULL, 1, NULL);

    print_uart("System ruszyl\r\n");
    return 0;
}

static void toggle_led(int led_index) {
    int ports[] = {LD1_PORT, LD2_PORT, LD3_PORT, LD4_PORT};
    int pins[] = {LD1_PIN, LD2_PIN, LD3_PIN, LD4_PIN};
    HAL_GPIO_TogglePin(ports[led_index], pins[led_index]);
}

static void print_uart(const char *msg) {
    if (!uart_handle || !msg) return;
    uint16_t len = (uint16_t)strlen(msg);

#if PROTECTION_MODE == 0
    HAL_UART_Transmit(uart_handle, (uint8_t *)msg, len, 1000);
#elif PROTECTION_MODE == 1
    if (xSemaphoreTake(uart_lock, SYNC_TIMEOUT) == pdTRUE) {
        HAL_UART_Transmit(uart_handle, (uint8_t *)msg, len, 1000);
        xSemaphoreGive(uart_lock);
    }
#elif PROTECTION_MODE == 2
    if (xSemaphoreTakeRecursive(uart_lock, SYNC_TIMEOUT) == pdTRUE) {
        HAL_UART_Transmit(uart_handle, (uint8_t *)msg, len, 1000);
        xSemaphoreGiveRecursive(uart_lock);
    }
#endif
}

static float get_noise(float amp) {
    static uint32_t seed = 0x12345678;
    seed = seed * 1664525 + 1013904223;
    int32_t r = (int32_t)(seed & 0xFFFF) - 32768;
    return ((float)r / 32767.0f) * amp;
}
