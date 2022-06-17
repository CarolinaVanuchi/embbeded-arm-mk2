#ifndef _MOTOR_BASE_
#define _MOTOR_BASE_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "generic_motor.h"

#define HORARIO_BASE (1)
#define ANTI_HORARIO_BASE (0)

#define ENABLE_BASE (0)
#define DISABLE_BASE (1)

#define LEDC_TIMER_BASE LEDC_TIMER_0
#define LEDC_MODE_BASE LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO_BASE (CONFIG_GPIO_MOTOR_BASE) 
#define LEDC_CHANNEL_BASE LEDC_CHANNEL_0
#define LEDC_DUTY_RES_BASE LEDC_TIMER_13_BIT 
#define LEDC_DUTY_BASE (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095

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

    ledc_timer_config_t ledc_timer_base = {
        .speed_mode = LEDC_MODE_BASE,
        .timer_num = LEDC_TIMER_BASE,
        .duty_resolution = LEDC_DUTY_RES_BASE,
        .freq_hz = frequency_base,
        .clk_cfg = LEDC_AUTO_CLK};

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_base));

    ledc_channel_config_t ledc_channel_base = {
        .speed_mode = LEDC_MODE_BASE,
        .channel = LEDC_CHANNEL_BASE,
        .timer_sel = LEDC_TIMER_BASE,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO_BASE,
        .duty = 0,
        .hpoint = 0};

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_base));

    gpio_set_level(CONFIG_GPIO_MOTOR_BASE_ENABLE, ENABLE_BASE);
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
                theta_base_value = get_new_theta(theta_base_value, theta_base_value_old, HORARIO_BASE, ANTI_HORARIO_BASE, CONFIG_GPIO_MOTOR_BASE_DIRECAO);

                theta_base_value_old = theta_base_value_new;
                end_sensor_base_check = 1;

                if (theta_base_value != 0)
                {
                    pwm_base(FREQUENCY_MAX);
                    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE_BASE, LEDC_CHANNEL_BASE, LEDC_DUTY_BASE));
                }

                ESP_LOGI(TAG_MOTOR_BASE, "theta original %f", theta_base_value_new);
                ESP_LOGI(TAG_MOTOR_BASE, "theta novo %f", theta_base_value);
                ESP_LOGI(TAG_MOTOR_BASE, "theta old %f", theta_base_value_old);
            }
            else
            {
                theta_base_value_old = theta_base_value;
                gpio_set_level(CONFIG_GPIO_MOTOR_BASE_DIRECAO, ANTI_HORARIO_BASE);
                pwm_base(FREQUENCY_MAX);
                ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE_BASE, LEDC_CHANNEL_BASE, LEDC_DUTY_BASE));
                task_on_base = 1;
            }
        }

        if (end_sensor_base_check == 1)
        {
            start_count_motor_base = 1;
            end_sensor_base_check = 0;
            start_timer_motor_base = esp_timer_get_time();
            end_motor = get_end_time(theta_base_value, FREQUENCY_MAX, 1, 1);
        }

        if (start_count_motor_base == 1)
        {
            current_timer_motor_base = esp_timer_get_time();
        }

        if (start_count_motor_base == 1 && ((current_timer_motor_base - start_timer_motor_base) >= end_motor))
        {
            ledc_stop(LEDC_MODE_BASE, LEDC_CHANNEL_BASE, 0);
            gpio_set_level(CONFIG_GPIO_MOTOR_BASE_ENABLE, DISABLE_BASE);
            start_count_motor_base = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#endif