/**
  ******************************************************************************
  * @file           : mx_freertos_app.c
  * @brief          : FreeRTOS with SSD1306 I2C Display
  ******************************************************************************
  */

#include "mx_freertos_app.h"
#include "ssd1306.h"
#include "mx_i2c1.h"
#include "stm32c5xx_hal.h"
#include "stm32c5xx_hal_gpio.h"
#include <stdio.h>
#include <string.h>
#include "stm32c5xx_hal_uart.h"

/* Defines */
#define Task1_stack_size  256U
#define Task2_stack_size  256U
hal_uart_handle_t *husart2;

/* LED GPIO */
#define LED_PORT HAL_GPIOA
#define LED_PIN  HAL_GPIO_PIN_5

/* Static variables */
static TaskHandle_t Task1_Handle;
static TaskHandle_t Task2_Handle;
static ssd1306_t oled_display;

/* Function prototypes */
static void display_task(void *pvParameters);
static void sensor_task(void *pvParameters);

static void uart_puts(const char *s)
{
  HAL_UART_Transmit(husart2, (uint8_t*)s, (uint16_t)strlen(s), 1000);
}

/**
  * @brief LED blink N times with duration
  * Used to signal which step failed
  */
static void blink_led(uint32_t count, uint32_t on_ms, uint32_t off_ms)
{
    for (uint32_t i = 0; i < count; i++)
    {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, HAL_GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, HAL_GPIO_PIN_SET);

    }
}

/**
  * @brief Initialize FreeRTOS tasks
  *
  * LED blink codes:
  * 1 slow blink = function entered
  * 2 slow blinks = I2C handle OK
  * 3 slow blinks = I2C test OK
  * 4 slow blinks = SSD1306 init OK
  * 5 slow blinks = display task created
  * 6 slow blinks = sensor task created
  * 7 slow blinks = COMPLETE
  *
  * Fast blinks = ERROR at that step
  */
int32_t app_synctasks_init(void)
{
    BaseType_t ret;
    husart2 = mx_usart2_uart_gethandle();
    hal_i2c_handle_t *hi2c;
    uint8_t init_ok;

    uart_puts("\n[APP] app_synctasks_init called\n");

    /* Signal: function entered */
    //blink_led(5, 500, 500);

    /* Step 1: Get I2C handle */
    uart_puts("[APP] Step 1: Getting I2C handle\n");
    hi2c = mx_i2c1_i2c_gethandle();

    if (hi2c == NULL)
    {
    	uart_puts("[APP] ERROR: I2C handle is NULL\n");
        blink_led(10, 100, 100);  /* Fast blinks = error */
        return -1;
    }

    uart_puts("[APP] I2C handle OK\n");
    blink_led(2, 500, 500);

    /* Step 2: Test I2C communication */
    uart_puts("[APP] Step 2: Testing I2C at 0x%02X\n");
    uint8_t test_cmd[2] = {0x00, 0xAE};
    hal_status_t status = HAL_I2C_MASTER_Transmit(hi2c, SSD1306_I2C_ADDR, test_cmd, 2, 100);

    if (status != HAL_OK)
    {
    	uart_puts("[APP] ERROR: I2C test failed (status=%d)\n");
        blink_led(10, 100, 100);  /* Fast blinks = error */
        return -1;
    }

    uart_puts("[APP] I2C test OK\n");
    blink_led(3, 500, 500);

    /* Step 3: Initialize SSD1306 */
    uart_puts("[APP] Step 3: Initializing SSD1306\n");
    memset(&oled_display, 0, sizeof(ssd1306_t));
    init_ok = ssd1306_init(&oled_display, hi2c);

    if (!init_ok)
    {
    	uart_puts("[APP] ERROR: SSD1306 init failed\n");
        blink_led(10, 100, 100);  /* Fast blinks = error */
        return -1;
    }

    uart_puts("[APP] SSD1306 init OK\n");
    blink_led(4, 500, 500);

    /* Step 4: Create display task */
    uart_puts("[APP] Step 4: Creating display task\n");
    ret = xTaskCreate(display_task, "DisplayTask", Task1_stack_size,
                      NULL, 2, &Task1_Handle);

    if (ret != pdPASS)
    {
        printf("[APP] ERROR: Display task creation failed\n");
        blink_led(10, 100, 100);  /* Fast blinks = error */
        return -1;
    }

    printf("[APP] Display task created\n");
    blink_led(5, 500, 500);

    /* Step 5: Create sensor task */
    printf("[APP] Step 5: Creating sensor task\n");
    ret = xTaskCreate(sensor_task, "SensorTask", Task2_stack_size,
                      NULL, 1, &Task2_Handle);

    if (ret != pdPASS)
    {
        printf("[APP] ERROR: Sensor task creation failed\n");
        blink_led(10, 100, 100);  /* Fast blinks = error */
        return -1;
    }

    printf("[APP] Sensor task created\n");
    blink_led(6, 500, 500);

    printf("[APP] Initialization COMPLETE\n");
    blink_led(7, 500, 500);

    return 0;
}

/**
  * @brief Display task
  */
static void display_task(void *pvParameters)
{
    (void)pvParameters;
    uint32_t counter = 0;
    char str[32];
    hal_status_t status;

    printf("[DISPLAY] Task started\n");

    for (;;)
    {
        printf("[DISPLAY] Update %lu\n", counter);

        ssd1306_clear(&oled_display);
        ssd1306_drawString(&oled_display, 0, 0, "STM32 C5", 1);
        ssd1306_drawString(&oled_display, 0, 10, "SSD1306", 1);
        snprintf(str, sizeof(str), "Cnt: %lu", counter++);
        ssd1306_drawString(&oled_display, 0, 24, str, 1);

        status = ssd1306_update(&oled_display);

        if (status == HAL_OK)
        {
            HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
  * @brief Sensor task
  */
static void sensor_task(void *pvParameters)
{
    (void)pvParameters;

    printf("[SENSOR] Task started\n");

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
