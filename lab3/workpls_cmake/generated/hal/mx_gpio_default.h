/**
  ******************************************************************************
  * @file           : mx_gpio_default.h
  * @brief          : Header for mx_gpio_default.c file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the mx_stm32c5xx_hal_drivers_license.md file
  * in the same directory as the generated code.
  * If no mx_stm32c5xx_hal_drivers_license.md file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_GPIO_DEFAULT_H
#define MX_GPIO_DEFAULT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "stm32_hal.h"
#include "mx_def.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/******************************************************************************/
/* Exported defines for gpio_default in HAL layer (SW instance MyGPIO_1) */
/******************************************************************************/

/* Primary aliases for GPIO PA5 pin */
#define PA5_PORT                                        HAL_GPIOA
#define PA5_PIN                                         HAL_GPIO_PIN_5
#define PA5_INIT_STATE                                  HAL_GPIO_PIN_RESET
#define PA5_ACTIVE_STATE                                HAL_GPIO_PIN_SET
#define PA5_INACTIVE_STATE                              HAL_GPIO_PIN_RESET

/* Secondary aliases for GPIO PA5 pin */
#define LD2_PORT                                        HAL_GPIOA
#define LD2_PIN                                         HAL_GPIO_PIN_5
#define LD2_INIT_STATE                                  HAL_GPIO_PIN_RESET
#define LD2_ACTIVE_STATE                                HAL_GPIO_PIN_SET
#define LD2_INACTIVE_STATE                              HAL_GPIO_PIN_RESET

#define LD1_PORT                                        HAL_GPIOA
#define LD1_PIN                                         HAL_GPIO_PIN_5
#define LD1_INIT_STATE                                  HAL_GPIO_PIN_RESET
#define LD1_ACTIVE_STATE                                HAL_GPIO_PIN_SET
#define LD1_INACTIVE_STATE                              HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PA6 pin */
#define PA6_PORT                                        HAL_GPIOA
#define PA6_PIN                                         HAL_GPIO_PIN_6
#define PA6_INIT_STATE                                  HAL_GPIO_PIN_RESET
#define PA6_ACTIVE_STATE                                HAL_GPIO_PIN_SET
#define PA6_INACTIVE_STATE                              HAL_GPIO_PIN_RESET

/* Secondary aliases for GPIO PA6 pin */
#define LD2_PORT                                        HAL_GPIOA
#define LD2_PIN                                         HAL_GPIO_PIN_6
#define LD2_INIT_STATE                                  HAL_GPIO_PIN_RESET
#define LD2_ACTIVE_STATE                                HAL_GPIO_PIN_SET
#define LD2_INACTIVE_STATE                              HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PA7 pin */
#define PA7_PORT                                        HAL_GPIOA
#define PA7_PIN                                         HAL_GPIO_PIN_7
#define PA7_INIT_STATE                                  HAL_GPIO_PIN_RESET
#define PA7_ACTIVE_STATE                                HAL_GPIO_PIN_SET
#define PA7_INACTIVE_STATE                              HAL_GPIO_PIN_RESET

/* Secondary aliases for GPIO PA7 pin */
#define LD3_PORT                                        HAL_GPIOA
#define LD3_PIN                                         HAL_GPIO_PIN_7
#define LD3_INIT_STATE                                  HAL_GPIO_PIN_RESET
#define LD3_ACTIVE_STATE                                HAL_GPIO_PIN_SET
#define LD3_INACTIVE_STATE                              HAL_GPIO_PIN_RESET

#define LD3_PORT                                        HAL_GPIOA
#define LD3_PIN                                         HAL_GPIO_PIN_7
#define LD3_INIT_STATE                                  HAL_GPIO_PIN_RESET
#define LD3_ACTIVE_STATE                                HAL_GPIO_PIN_SET
#define LD3_INACTIVE_STATE                              HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PC6 pin */
#define PC6_PORT                                        HAL_GPIOC
#define PC6_PIN                                         HAL_GPIO_PIN_6
#define PC6_INIT_STATE                                  HAL_GPIO_PIN_RESET
#define PC6_ACTIVE_STATE                                HAL_GPIO_PIN_SET
#define PC6_INACTIVE_STATE                              HAL_GPIO_PIN_RESET

/* Secondary aliases for GPIO PC6 pin */
#define LD4_PORT                                        HAL_GPIOC
#define LD4_PIN                                         HAL_GPIO_PIN_6
#define LD4_INIT_STATE                                  HAL_GPIO_PIN_RESET
#define LD4_ACTIVE_STATE                                HAL_GPIO_PIN_SET
#define LD4_INACTIVE_STATE                              HAL_GPIO_PIN_RESET

#define LD4_PORT                                        HAL_GPIOC
#define LD4_PIN                                         HAL_GPIO_PIN_6
#define LD4_INIT_STATE                                  HAL_GPIO_PIN_RESET
#define LD4_ACTIVE_STATE                                HAL_GPIO_PIN_SET
#define LD4_INACTIVE_STATE                              HAL_GPIO_PIN_RESET

/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/******************************************************************************/
/* Exported functions for gpio_default in HAL layer (SW instance MyGPIO_1) */
/******************************************************************************/
/**
  * @brief mx_gpio_default init function
  * This function configures the hardware resources used in this example
  * @retval 0  GPIO group correctly initialized
  * @retval -1 Issue detected during GPIO group initialization
  */
system_status_t mx_gpio_default_init(void);

/**
  * @brief  De-initialize gpio_default instance.
  */
system_status_t mx_gpio_default_deinit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_GPIO_DEFAULT_H */
