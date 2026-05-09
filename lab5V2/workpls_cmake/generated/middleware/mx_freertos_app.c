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

static hal_uart_handle_t *uart_handle;
static hal_adc_handle_t  *pADC;

#define VREFPLUS_APPLI    3300U
#define TIME_OUT          10U

static uint16_t TempDegreeCelsius;
static uint16_t adc_raw;

static void toggle_led(int led_index) {
    int ports[] = {LD1_PORT, LD2_PORT, LD3_PORT, LD4_PORT};
    int pins[] = {LD1_PIN, LD2_PIN, LD3_PIN, LD4_PIN};
    HAL_GPIO_TogglePin(ports[led_index], pins[led_index]);
}

static void print_uart(const char *msg) {
    if (!uart_handle || !msg) return;
    uint16_t len = (uint16_t)strlen(msg);
    HAL_UART_Transmit(uart_handle, (uint8_t *)msg, len, 1000);

}

hal_uart_handle_t *mx_usart2_uart_gethandle(void);




static void task_a_sensor(void *params)
{
  (void)params;

  // ADC activation + calibration (do this ONCE)
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

      char buf[128];
      // raw printed hex like the ST example; temp as decimal
      snprintf(buf, sizeof(buf),
               "ADC raw=0x%04" PRIx16 " Temp=%" PRId16 " C\r\n",
               adc_raw, TempDegreeCelsius);
      print_uart(buf);
    }
    else
    {
      print_uart("ADC conversion error\r\n");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// Task B
static void task_b_calc(void *params) {


    while (1) {

    }
}

static void task_c_monitor(void *params) {

    while (1) {

    }
}

int32_t app_synctasks_init(void)
{
  uart_handle = mx_usart2_uart_gethandle();

  // This comes from generated code; in the ST example it's mx_example_adc_init()
  pADC = mx_example_adc_init();
  if (pADC == NULL) {
    print_uart("ADC init returned NULL\r\n");
    return -1;
  }

  xTaskCreate(task_a_sensor, "TaskA", 512, NULL, 3, NULL);
  xTaskCreate(task_b_calc,   "TaskB", 512, NULL, 2, NULL);
  xTaskCreate(task_c_monitor,"TaskC", 512, NULL, 1, NULL);

  print_uart("System ruszyl\r\n");
  return 0;
}



