#ifndef _MOTOR_
#define _MOTOR_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/mcpwm.h"

static xQueueHandle theta_base = NULL;

void init_motor_base(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE);

    theta_base = xQueueCreate(1, sizeof(double));
}

void pwm_base(uint8_t frequency_base)
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, CONFIG_GPIO_MOTOR_BASE);

    mcpwm_config_t pwm_config = {
        .frequency = frequency_base,
        .cmpr_a = 0,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };

    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

    gpio_set_level(CONFIG_GPIO_MOTOR_BASE_DIRECAO, 0);
    gpio_set_level(CONFIG_GPIO_MOTOR_BASE_ENABLE, 0);
}

double get_end_time(double angle_base, int hertz_base) {
    double passos = ((1600 * angle_base) / 360);
    return (passos/hertz_base)*100000;
}
static void task_motor_base(void *arg)
{
    init_motor_base();
    double theta_base_value;
    int64_t start_timer = 0;
    int64_t current_timer = 0;
    double end_motor = 0;

    while (1)
    {
        if (xQueueReceiveFromISR(theta_base, &theta_base_value, 100))
        {
            ESP_LOGI("base angles", "%lf...", theta_base_value);
            pwm_base(250);
            start_timer = esp_timer_get_time();
            end_motor = get_end_time(theta_base_value, 250);
        }
        current_timer = esp_timer_get_time();

        
        ESP_LOGI("start_timer", "value: %lld us", start_timer);
        ESP_LOGI("current_timer", "value: %lld us", current_timer);
        ESP_LOGI("end", "value: %lf us", end_motor);
    }
}

#endif