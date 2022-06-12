#ifndef _MOTOR_
#define _MOTOR_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

static xQueueHandle theta_base = NULL;

void init_motor_base(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE, GPIO_MODE_OUTPUT);

    gpio_set_level(CONFIG_GPIO_MOTOR_BASE_DIRECAO, 0);
    gpio_set_level(CONFIG_GPIO_MOTOR_BASE_ENABLE, 0);

    theta_base = xQueueCreate(1, sizeof(double));
}

// pdMS_TO_TICKS passa em millisegundos
static void task_motor_base(void *arg)
{
    init_motor_base();
    double theta_base_value;

    while (1)
    {
        if (xQueueReceiveFromISR(theta_base, &theta_base_value, 100))
        {
            ESP_LOGI("base angles", "%lf...", theta_base_value);
        }

        gpio_set_level(CONFIG_GPIO_MOTOR_BASE, 1);
        vTaskDelay(pdMS_TO_TICKS(10));

        gpio_set_level(CONFIG_GPIO_MOTOR_BASE, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#endif