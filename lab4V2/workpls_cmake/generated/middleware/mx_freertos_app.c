#include "stm32c5xx_hal.h"
#include "FreeRTOS.h"
#include "stm32c5xx_hal_gpio.h"
#include "stm32c5xx_hal_uart.h"
#include "task.h"
#include "semphr.h"
#include "mx_gpio_default.h"
#include "mx_hal_def.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32c5xx_hal_adc.h"
#include "mx_adc1.h"
#include <inttypes.h>
#include "queue.h"

QueueHandle_t sensorQueue;
static hal_uart_handle_t *uart_handle;
static hal_adc_handle_t  *pADC;

#define VREFPLUS_APPLI    3300U
#define TIME_OUT          10U
#define RNG_GENERATION_TIMEOUT 10U

hal_rng_handle_t *pRNG;
uint32_t RandomNumbers[1] = {0};
float lastpi = 0.f;
uint16_t lasttemp = 0;

typedef struct {
    float pi_estimate;
    uint16_t temperature_celsius;
} SensorPiResult_t;

static void print_uart(const char *msg) {
    if (!uart_handle || !msg) return;
    uint16_t len = (uint16_t)strlen(msg);
    HAL_UART_Transmit(uart_handle, (uint8_t *)msg, len, 1000);
}

static void task_a_sensor(void *params)
{
    static uint16_t TempDegreeCelsius;
    static uint16_t adc_raw;

    if (HAL_ADC_Start(pADC) != HAL_OK) {
        print_uart("ADC start failed\r\n");
        vTaskDelete(NULL);
    }
    if (HAL_ADC_Calibrate(pADC) != HAL_OK) {
        print_uart("ADC calibrate failed\r\n");
        vTaskDelete(NULL);
    }

    for (;;)
    {
        if (HAL_ADC_REG_StartConv(pADC) == HAL_OK &&
            HAL_ADC_REG_PollForConv(pADC, TIME_OUT) == HAL_OK)
        {
            adc_raw = HAL_ADC_REG_ReadConversionData(pADC);
            TempDegreeCelsius = HAL_ADC_CALC_TEMPERATURE(
                VREFPLUS_APPLI,
                adc_raw,
                HAL_ADC_RESOLUTION_12_BIT);

            char buf[96];
            snprintf(buf, sizeof(buf),
                     "ADC raw=0x%04" PRIx16 " Temp=%" PRId16 "C\r\n",
                     adc_raw, TempDegreeCelsius);
            //print_uart(buf);

            if (lasttemp != TempDegreeCelsius) {
                lasttemp = TempDegreeCelsius;
                SensorPiResult_t result;
                result.temperature_celsius = TempDegreeCelsius;
                result.pi_estimate = lastpi;
                if (xQueueSend(sensorQueue, &result, 0) != pdPASS) {
                    print_uart("Failed to send to queue\r\n");
                }
            }
        } else {
            print_uart("ADC conversion error\r\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void task_pi(void *params) {
    for (;;) {
        if (HAL_RNG_GenerateRandomNumber(pRNG, RandomNumbers, 1, RNG_GENERATION_TIMEOUT) != HAL_OK) {
            print_uart("RNG failed\r\n");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        int iter = RandomNumbers[0] % 100;
        if (iter <= 0) iter = 10;

        if (HAL_RNG_GenerateRandomNumber(pRNG, RandomNumbers, 1, RNG_GENERATION_TIMEOUT) != HAL_OK) {
            print_uart("RNG failed\r\n");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        int radius = RandomNumbers[0] % 10;
        if (radius <= 0) radius = 5;

        int count = 0;

        for (int i = 0; i < iter; i++) {
            if (HAL_RNG_GenerateRandomNumber(pRNG, RandomNumbers, 1, RNG_GENERATION_TIMEOUT) != HAL_OK) continue;
            int pointX = (RandomNumbers[0] % 21) - 10;
            if (HAL_RNG_GenerateRandomNumber(pRNG, RandomNumbers, 1, RNG_GENERATION_TIMEOUT) != HAL_OK) continue;
            int pointY = (RandomNumbers[0] % 21) - 10;

            if ((pointX * pointX + pointY * pointY) <= (radius * radius)) count++;
        }
        float ratio = (float)count / (float)iter;
        float estimate_pi = 4.0f * ratio;
        lastpi = estimate_pi;

        int whole = (int)estimate_pi;
        int frac = (int)((estimate_pi - whole) * 10000);
        if (frac < 0) frac = -frac;
        char buf[80];
       snprintf(buf, sizeof(buf), "Monte Carlo PI: %d.%04d (N=%d, R=%d)\r\n", whole, frac, iter, radius);
        //print_uart(buf);

        SensorPiResult_t result;
        result.temperature_celsius = lasttemp;
        result.pi_estimate = estimate_pi;
        if (xQueueSend(sensorQueue, &result, 0) != pdPASS) {
            print_uart("Failed to send Pi to queue\r\n");
        }

        if (HAL_RNG_GenerateRandomNumber(pRNG, RandomNumbers, 1, RNG_GENERATION_TIMEOUT) != HAL_OK)
            vTaskDelay(pdMS_TO_TICKS(1000));
        else {
            int sleep = RandomNumbers[0] % 5000;
            if (sleep < 100) sleep = 100;
            vTaskDelay(pdMS_TO_TICKS(sleep));
        }
    }
}

static void task_b_calc(void *params) {
    for (;;) {
        if (HAL_GPIO_ReadPin(PC13_PORT, PC13_PIN)) {
            UBaseType_t num_msgs = uxQueueMessagesWaiting(sensorQueue);
            if (num_msgs == 0) {
                print_uart("Queue empty\r\n");
            } else {
                for (UBaseType_t i = 0; i < num_msgs; i++) {
                    SensorPiResult_t receivedResult;
                    if (xQueueReceive(sensorQueue, &receivedResult, 0) == pdPASS) {
                        int whole = (int)receivedResult.pi_estimate;
                        int frac = (int)((receivedResult.pi_estimate - whole) * 10000);
                        if (frac < 0) frac = -frac;

                        char buf[128];
                        snprintf(buf, sizeof(buf),
                            "Q: Temp=%uC, Pi=%d.%04d\r\n",
                            receivedResult.temperature_celsius,
                            whole, frac);
                        print_uart(buf);
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int32_t app_synctasks_init(void)
{
    uart_handle = mx_usart2_uart_gethandle();
    pRNG = mx_rng_gethandle();
    sensorQueue = xQueueCreate(20, sizeof(SensorPiResult_t));
    if (sensorQueue == NULL) {
        print_uart("Queue creation failed\r\n");
        return -1;
    }

    pADC = mx_adc1_init();
    if (pADC == NULL) {
        print_uart("ADC init returned NULL\r\n");
        return -1;
    }

    xTaskCreate(task_a_sensor, "TaskA", 512, NULL, 1, NULL);
    xTaskCreate(task_b_calc,   "TaskB", 512, NULL, 2, NULL);
    xTaskCreate(task_pi, "TaskC", 512, NULL, 1, NULL);

    print_uart("System ruszyl\r\n");
    return 0;
}
