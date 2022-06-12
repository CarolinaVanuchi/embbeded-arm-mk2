#ifndef _MOTOR_LEFT_
#define _MOTOR_LEFT_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

static void task_motor_left(void *arg)
{
    while (1)
    {
        gpio_set_level(CONFIG_GPIO_MOTOR_ESQUERDO, 1);
        vTaskDelay(pdMS_TO_TICKS(10));

        gpio_set_level(CONFIG_GPIO_MOTOR_ESQUERDO, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#endif