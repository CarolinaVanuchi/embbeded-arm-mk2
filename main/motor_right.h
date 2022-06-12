#ifndef _MOTOR_RIGHT_
#define _MOTOR_RIGHT_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

void init_motor_right(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_DIREITO_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_DIREITO_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_DIREITO_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_DIREITO_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_DIREITO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_DIREITO, GPIO_MODE_OUTPUT);
}

static void task_motor_right(void *arg)
{
    init_motor_right();
    
    while (1)
    {
        gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO, 1);
        vTaskDelay(pdMS_TO_TICKS(10));

        gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#endif