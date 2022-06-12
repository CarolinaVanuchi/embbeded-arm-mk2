#ifndef _MOTOR_
#define _MOTOR_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

static void task_motor_base(void *arg)
{
    while (1)
    {
        gpio_set_level(CONFIG_GPIO_MOTOR_BASE, 1);
        vTaskDelay(10 / portTICK_PERIOD_MS);

        gpio_set_level(CONFIG_GPIO_MOTOR_BASE, 0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

#endif