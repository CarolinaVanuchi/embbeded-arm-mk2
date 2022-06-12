#ifndef _MOTOR_LEFT_
#define _MOTOR_LEFT_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

void init_motor_left(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_ESQUERDO_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_ESQUERDO_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_ESQUERDO_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_ESQUERDO_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_ESQUERDO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_ESQUERDO, GPIO_MODE_OUTPUT);
}

static void task_motor_left(void *arg)
{
    init_motor_left();
    
    while (1)
    {
        gpio_set_level(CONFIG_GPIO_MOTOR_ESQUERDO, 1);
        vTaskDelay(pdMS_TO_TICKS(10));

        gpio_set_level(CONFIG_GPIO_MOTOR_ESQUERDO, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#endif