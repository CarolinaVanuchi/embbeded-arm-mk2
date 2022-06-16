#ifndef _MOTOR_
#define _MOTOR_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/mcpwm.h"

#define FREQUENCY_MAX_BASE (250)

#define HORARIO_BASE (1)
#define ANTI_HORARIO_BASE (0)

#define ENABLE_BASE (0)
#define DISABLE_BASE (1)

static const char *TAG_MOTOR_BASE = "MOTOR BASE";

static xQueueHandle theta_base = NULL;
uint8_t end_sensor_base_check = 0;

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
    gpio_set_level(CONFIG_GPIO_MOTOR_BASE_ENABLE, ENABLE_BASE);
}

double get_new_theta(double new_theta_base, double old_theta_base)
{
    double aux_base = new_theta_base - old_theta_base;
    if (aux_base > 0)
    {
        gpio_set_level(CONFIG_GPIO_MOTOR_BASE_DIRECAO, HORARIO_BASE);
        return aux_base;
    }
    else if (aux_base < 0)
    {
        gpio_set_level(CONFIG_GPIO_MOTOR_BASE_DIRECAO, ANTI_HORARIO_BASE);
        return (aux_base * (-1));
    }

    return 0;
}
double get_end_time(double angle_base, int hertz_base)
{
    double passos = ((200 * 8 * angle_base) / 360);
    return (passos / hertz_base) * 1000000;
}
static void task_motor_base(void *arg)
{
    init_motor_base();
    uint8_t task_on_base = 0;

    double theta_base_value_new;
    double theta_base_value_old;

    double theta_base_value;
    int64_t start_timer_motor_base = 0;
    int64_t current_timer_motor_base = 0;
    double end_motor = 0;
    uint8_t start_count_motor_base = 0;

    while (1)
    {
        
        if (xQueueReceiveFromISR(theta_base, &theta_base_value, 100))
        {
            ESP_LOGI(TAG_MOTOR_BASE, "%lf...", theta_base_value);
            start_count_motor_base = 0;

            if (task_on_base == 1)
            {
                theta_base_value_new = theta_base_value;
                theta_base_value = get_new_theta(theta_base_value, theta_base_value_old);

                theta_base_value_old = theta_base_value_new;
                end_sensor_base_check = 1;

                if (theta_base_value != 0)
                    pwm_base(FREQUENCY_MAX_BASE);

                ESP_LOGI(TAG_MOTOR_BASE, "theta original %f", theta_base_value_new);
                ESP_LOGI(TAG_MOTOR_BASE, "theta novo %f", theta_base_value);
                ESP_LOGI(TAG_MOTOR_BASE, "theta old %f", theta_base_value_old);
            }
            else
            {
                theta_base_value_old = theta_base_value;
                gpio_set_level(CONFIG_GPIO_MOTOR_BASE_DIRECAO, ANTI_HORARIO_BASE);
                pwm_base(FREQUENCY_MAX_BASE);
                task_on_base = 1;
            }
        }

        if (end_sensor_base_check == 1)
        {
            start_count_motor_base = 1;
            end_sensor_base_check = 0;
            start_timer_motor_base = esp_timer_get_time();
            end_motor = get_end_time(theta_base_value, FREQUENCY_MAX_BASE);
        }

        if (start_count_motor_base == 1)
        {
            current_timer_motor_base = esp_timer_get_time();
        }

        if (start_count_motor_base == 1 && ((current_timer_motor_base - start_timer_motor_base) >= end_motor))
        {
            mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
            gpio_set_level(CONFIG_GPIO_MOTOR_BASE_ENABLE, DISABLE_BASE);
            start_count_motor_base = 0;
        }
         vTaskDelay(100);
    }
}

#endif