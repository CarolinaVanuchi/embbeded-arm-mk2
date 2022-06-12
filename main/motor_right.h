#ifndef _MOTOR_RIGHT_
#define _MOTOR_RIGHT_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

static void task_motor_right(void *arg)
{
    while (1)
    {
        gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO, 1);
        vTaskDelay(10 / portTICK_PERIOD_MS);

        gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO, 0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

#endif