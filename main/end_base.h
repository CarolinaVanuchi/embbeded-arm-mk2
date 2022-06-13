#ifndef _END_BASE_
#define _END_BASE_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "sensor.h"

void init_end_base(void)
{
    gpio_config_t end_base_sensor = {.mode = GPIO_MODE_INPUT, .pin_bit_mask = (1ULL << CONFIG_GPIO_END_BASE)};
    gpio_config(&end_base_sensor);
}

static void task_end_base(void *arg)
{
    uint8_t input_end_base_sensor;

    while (1)
    {
        input_end_base_sensor = gpio_get_level(CONFIG_GPIO_END_BASE);
        if (input_end_base_sensor == 0)
        {
           xQueueSend(gpio_end_motor_base, (void *)&input_end_base_sensor, 1000);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

#endif
