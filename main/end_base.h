#ifndef _END_BASE_
#define _END_BASE_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "sensor.h"

/**
 * @brief Inicializa os pinos dos sensores da base
 * 
 */
void init_end_base(void)
{
    gpio_config_t end_base_sensor = {.mode = GPIO_MODE_INPUT, .pin_bit_mask = (1ULL << CONFIG_GPIO_END_BASE)};
    gpio_config(&end_base_sensor);
}

/**
 * @brief Task que observa o sensor da base
 * 
 * @param arg 
 */
static void task_end_base(void *arg)
{
    while (1)
    {
        if (gpio_get_level(CONFIG_GPIO_END_BASE) == 0)
        {
           gpio_end_motor_base = true;
           
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#endif
