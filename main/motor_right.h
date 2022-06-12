#ifndef _MOTOR_RIGHT_
#define _MOTOR_RIGHT_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

static xQueueHandle theta_right = NULL;

void init_motor_right(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_DIREITO_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_DIREITO_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_DIREITO_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_DIREITO_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_DIREITO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_DIREITO, GPIO_MODE_OUTPUT);

    gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO_DIRECAO, 0);
    gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO_ENABLE, 0);

    theta_right = xQueueCreate(1, sizeof(double));
}

static void task_motor_right(void *arg)
{
    init_motor_right();
    double theta_right_value;

    while (1)
    {

        if (xQueueReceiveFromISR(theta_right, &theta_right_value, 100))
        {
            ESP_LOGI("right angles", "%lf...", theta_right_value);
        }

        gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO, 1);
        vTaskDelay(pdMS_TO_TICKS(10));

        gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#endif