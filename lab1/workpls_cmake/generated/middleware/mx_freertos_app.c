#include "mx_freertos_app.h"
#include "stm32c5xx_hal.h"
#include "stm32c5xx_hal_gpio.h"
#include "stm32c5xx_hal_uart.h"

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>   /* snprintf */

/* Private define ------------------------------------------------------------*/
#define Task1_stack_size  256U
#define Task2_stack_size  256U
#define RX_LINE_MAX 64

/* Morse timing (ms) */
#define MORSE_UNIT_MS            120U
#define DOT_MS                   (MORSE_UNIT_MS)
#define DASH_MS                  (3U * MORSE_UNIT_MS)
#define INTRA_ELEMENT_GAP_MS     (MORSE_UNIT_MS)
#define LETTER_GAP_MS            (3U * MORSE_UNIT_MS)
#define WORD_GAP_MS              (7U * MORSE_UNIT_MS)

/* Private variables ---------------------------------------------------------*/
hal_uart_handle_t *husart2;
static TaskHandle_t Task1_Handle;
static TaskHandle_t Task2_Handle;

/* Private functions prototype -----------------------------------------------*/
static void function1(void *pvParameters);
static void function2(void *pvParameters);

/* ---------- UART helpers ---------- */
static void uart_puts(const char *s)
{
  /* NOTE: This assumes husart2 is compatible with HAL_UART_Transmit(). */
  HAL_UART_Transmit(husart2, (uint8_t*)s, (uint16_t)strlen(s), 1000);
}

/* ---------- LED helpers (your environment uses LL pin states) ---------- */
static void led_on(void)
{
  HAL_GPIO_WritePin(LD2_PORT, LD2_PIN, LL_GPIO_PIN_SET);
}
static void led_off(void)
{
  HAL_GPIO_WritePin(LD2_PORT, LD2_PIN, LL_GPIO_PIN_RESET);
}

static void morse_dot(void)
{
  led_on();
  vTaskDelay(pdMS_TO_TICKS(DOT_MS));
  led_off();
  vTaskDelay(pdMS_TO_TICKS(INTRA_ELEMENT_GAP_MS));
}
static void morse_dash(void)
{
  led_on();
  vTaskDelay(pdMS_TO_TICKS(DASH_MS));
  led_off();
  vTaskDelay(pdMS_TO_TICKS(INTRA_ELEMENT_GAP_MS));
}

static const char *morse_for_char(char c)
{
  if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');

  switch (c)
  {
    case 'A': return ".-";
    case 'B': return "-...";
    case 'C': return "-.-.";
    case 'D': return "-..";
    case 'E': return ".";
    case 'F': return "..-.";
    case 'G': return "--.";
    case 'H': return "....";
    case 'I': return "..";
    case 'J': return ".---";
    case 'K': return "-.-";
    case 'L': return ".-..";
    case 'M': return "--";
    case 'N': return "-.";
    case 'O': return "---";
    case 'P': return ".--.";
    case 'Q': return "--.-";
    case 'R': return ".-.";
    case 'S': return "...";
    case 'T': return "-";
    case 'U': return "..-";
    case 'V': return "...-";
    case 'W': return ".--";
    case 'X': return "-..-";
    case 'Y': return "-.--";
    case 'Z': return "--..";

    case '0': return "-----";
    case '1': return ".----";
    case '2': return "..---";
    case '3': return "...--";
    case '4': return "....-";
    case '5': return ".....";
    case '6': return "-....";
    case '7': return "--...";
    case '8': return "---..";
    case '9': return "----.";

    case ' ': return " "; /* word gap marker */

    default:  return NULL;
  }
}

static void morse_blink_message(const char *msg)
{
  for (size_t i = 0; msg[i] != '\0'; i++)
  {
    const char *pat = morse_for_char(msg[i]);

    if (pat == NULL)
    {
      vTaskDelay(pdMS_TO_TICKS(WORD_GAP_MS));
      continue;
    }

    if (pat[0] == ' ' && pat[1] == '\0')
    {
      vTaskDelay(pdMS_TO_TICKS(WORD_GAP_MS));
      continue;
    }

    for (size_t j = 0; pat[j] != '\0'; j++)
    {
      if (pat[j] == '.') morse_dot();
      else if (pat[j] == '-') morse_dash();
    }

    /* We already waited 1 unit after last dot/dash.
       Convert to 3-unit letter gap total. */
    vTaskDelay(pdMS_TO_TICKS(LETTER_GAP_MS - INTRA_ELEMENT_GAP_MS));
  }
}

/* Reverse lookup: pattern -> character */
static char morse_to_char(const char *pat)
{
  if (strcmp(pat, ".-") == 0)   return 'A';
  if (strcmp(pat, "-...") == 0) return 'B';
  if (strcmp(pat, "-.-.") == 0) return 'C';
  if (strcmp(pat, "-..") == 0)  return 'D';
  if (strcmp(pat, ".") == 0)    return 'E';
  if (strcmp(pat, "..-.") == 0) return 'F';
  if (strcmp(pat, "--.") == 0)  return 'G';
  if (strcmp(pat, "....") == 0) return 'H';
  if (strcmp(pat, "..") == 0)   return 'I';
  if (strcmp(pat, ".---") == 0) return 'J';
  if (strcmp(pat, "-.-") == 0)  return 'K';
  if (strcmp(pat, ".-..") == 0) return 'L';
  if (strcmp(pat, "--") == 0)   return 'M';
  if (strcmp(pat, "-.") == 0)   return 'N';
  if (strcmp(pat, "---") == 0)  return 'O';
  if (strcmp(pat, ".--.") == 0) return 'P';
  if (strcmp(pat, "--.-") == 0) return 'Q';
  if (strcmp(pat, ".-.") == 0)  return 'R';
  if (strcmp(pat, "...") == 0)  return 'S';
  if (strcmp(pat, "-") == 0)    return 'T';
  if (strcmp(pat, "..-") == 0)  return 'U';
  if (strcmp(pat, "...-") == 0) return 'V';
  if (strcmp(pat, ".--") == 0)  return 'W';
  if (strcmp(pat, "-..-") == 0) return 'X';
  if (strcmp(pat, "-.--") == 0) return 'Y';
  if (strcmp(pat, "--..") == 0) return 'Z';
  return '?';
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

  /* Decoder task higher priority than blinker */
  ret = xTaskCreate(function1, "Task1", Task1_stack_size,
                    (void*)NULL, 1, &Task1_Handle);
  ret = xTaskCreate(function2, "Task2", Task2_stack_size,
                    (void*)NULL, 2, &Task2_Handle);

  if (ret != pdPASS)
  {
    return -1;
  }

  return 0;
}

/* Task1 = transmitter: blink SOS forever */
static void function1(void *pvParameters)
{
  (void)pvParameters;

  uart_puts("TX: blinking SOS on LD2\r\n");

  for (;;)
  {
    morse_blink_message("SOS");

    /* Gap between repeats */
    vTaskDelay(pdMS_TO_TICKS(WORD_GAP_MS + 1000U));
  }
}

/* Task2 = receiver: decode INPUT pin (shorted to LD2 output) */
static void function2(void *pvParameters)
{
  (void)pvParameters;

  uart_puts("RX: decoding from INPUT pin\r\n");

  char pat[8];
  uint32_t pat_len = 0;

  /* Assumption for this wiring: LD2 pin high = LED on = "signal present" */
  uint32_t prev = (uint32_t)HAL_GPIO_ReadPin(INPUT_PORT, INPUT_PIN);
  TickType_t last_edge = xTaskGetTickCount();

  for (;;)
  {
    uint32_t cur = (uint32_t)HAL_GPIO_ReadPin(INPUT_PORT, INPUT_PIN);

    if (cur != prev)
    {
      TickType_t now = xTaskGetTickCount();
      uint32_t dt_ms = (uint32_t)((now - last_edge) * portTICK_PERIOD_MS);
      last_edge = now;

      /* HIGH->LOW: ON time ended => dot or dash */
      if (prev == LL_GPIO_PIN_SET && cur == LL_GPIO_PIN_RESET)
      {
        if (dt_ms < (DOT_MS + DASH_MS) / 2U)
        {
          if (pat_len < (sizeof(pat) - 1U)) pat[pat_len++] = '.';
        }
        else
        {
          if (pat_len < (sizeof(pat) - 1U)) pat[pat_len++] = '-';
        }
        pat[pat_len] = '\0';
      }
      /* LOW->HIGH: gap ended => if long enough, end of letter/word */
      else if (prev == LL_GPIO_PIN_RESET && cur == LL_GPIO_PIN_SET)
      {
        if (dt_ms >= LETTER_GAP_MS)
        {
          if (pat_len > 0U)
          {
            char decoded = morse_to_char(pat);

            char out[64];
            snprintf(out, sizeof(out), "pat=%s -> %c\r\n", pat, decoded);
            uart_puts(out);

            pat_len = 0;
            pat[0] = '\0';
          }

          if (dt_ms >= WORD_GAP_MS)
          {
            uart_puts("[word gap]\r\n");
          }
        }
      }

      prev = cur;
    }

    vTaskDelay(pdMS_TO_TICKS(2));
  }
}
