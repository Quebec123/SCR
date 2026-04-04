#include "mx_freertos_app.h"
#include "stm32c5xx_hal.h"
#include "stm32c5xx_hal_gpio.h"
#include "stm32c5xx_hal_uart.h"
#include "stm32c5xx_hal_rng.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Private define ------------------------------------------------------------*/
#define Task1_stack_size  256U
#define Task2_stack_size  256U
#define Task3_stack_size  256U
#define Task4_stack_size  256U
#define Task5_stack_size  256U
#define RX_LINE_MAX 64

#define RNG_NUMBERS   8U      /* Number of 32-bit random numbers that will be generated */
#define RNG_GENERATION_TIMEOUT   10U            /* Timeout for generation in millisecond */

/* Private variables ---------------------------------------------------------*/
hal_uart_handle_t *husart2;
static TaskHandle_t Task1_Handle;
static TaskHandle_t Task2_Handle;
static TaskHandle_t Task3_Handle;
static TaskHandle_t Task4_Handle;
TaskHandle_t taskArray[4];
bool isOn[4]={1,1,1,1};
static TaskHandle_t Task5_Handle;

hal_rng_handle_t *pRNG;  /* pointer referencing the RNG handle from the generated code */
uint32_t RandomNumbers[RNG_NUMBERS] = {0}; /* array to store the generated random numbers */

/* Private functions prototype -----------------------------------------------*/
static void function1(void *pvParameters);
static void function2(void *pvParameters);
static void function3(void *pvParameters);
static void function4(void *pvParameters);
static void function5(void *pvParameters);
static void uart_puts(const char *s)
{
  HAL_UART_Transmit(husart2, (uint8_t*)s, (uint16_t)strlen(s), 1000);
}
//łatwiejsza obsługa ledów
static void led_toggle(int led)
{
	int arr1[4] = {LD1_PORT, LD2_PORT, LD3_PORT, LD4_PORT};
	int arr2[4] = {LD1_PIN, LD2_PIN, LD3_PIN, LD4_PIN};
  HAL_GPIO_TogglePin(arr1[led], arr2[led]);
}


uint32_t rng_0_3(uint32_t timeout_ms)
{
    uint32_t r;

    if (HAL_RNG_GenerateRandomNumber(pRNG, RandomNumbers, RNG_NUMBERS, RNG_GENERATION_TIMEOUT) != HAL_OK) {
        r = HAL_GetTick();
    }

    return (r & 0x3u); // 0..3
}

/**
  * @brief Initializes FreeRTOS kernel objects.
  */
int32_t app_synctasks_init (void)
{
  BaseType_t ret;

  husart2 = mx_usart2_uart_gethandle();
  if (husart2 == NULL)
  {
    return -1;
  }

  ret = xTaskCreate(function1, "Task1", Task1_stack_size,
                    (void*)NULL, 1, &Task1_Handle);
  ret = xTaskCreate(function2, "Task2", Task2_stack_size,
                    (void*)NULL, 1, &Task2_Handle); //musi być wyższy piorytet
  ret = xTaskCreate(function3, "Task3", Task3_stack_size,
                    (void*)NULL, 1, &Task3_Handle); //musi być wyższy piorytet
  ret = xTaskCreate(function4, "Task4", Task4_stack_size,
                    (void*)NULL, 1, &Task4_Handle); //musi być wyższy piorytet
  ret = xTaskCreate(function5, "Task5", Task5_stack_size,
                    taskArray, 2, &Task5_Handle); //musi być wyższy piorytet

  taskArray[0] = Task1_Handle;
  taskArray[1] = Task2_Handle;
  taskArray[2] = Task3_Handle;
  taskArray[3] = Task4_Handle;
  if (ret != pdPASS)
  {
    return -1;
  }

  return 0;
}
//ciągłe nadawanie na diodzie, jest zrobion "zwaercie" by przesłać sygnały na drugi pin
static void function1(void *pvParameters)
{
  (void)pvParameters;

  uart_puts("TX: blinking SOS on LD2\r\n");

  for (;;)
  {
	  led_toggle(0);
    vTaskDelay(10);
  }
}
//dekodowanie morsa i wypisanie na serialporcie
static void function2(void *pvParameters)
{
  (void)pvParameters;



  for (;;)
  {
	  led_toggle(1);
    vTaskDelay(10);
  }
}
static void function3(void *pvParameters)
{
  (void)pvParameters;



  for (;;)
  {
	  led_toggle(2);
    vTaskDelay(10);
  }
}
static void function4(void *pvParameters)
{
  (void)pvParameters;



  for (;;)
  {
	  led_toggle(3);
    vTaskDelay(10);
  }
}
static void function5(void *pvParameters)
{
  (void)pvParameters;

  for (;;)
  {
	  int current=rng_0_3(100);
	  if(isOn[current]!=0){
		  vTaskSuspend(taskArray[current]);
		  vTaskDelay((current+10)*100);
		  vTaskResume(taskArray[current]);
	  }
    vTaskDelay(10);
  }
}
