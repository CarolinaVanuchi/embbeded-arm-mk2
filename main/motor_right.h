#ifndef _MOTOR_RIGHT_
#define _MOTOR_RIGHT_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include "generic_motor.h"
#include "uart.h"

#define HORARIO_RIGHT (1)
#define ANTI_HORARIO_RIGHT (0)

#define ENABLE_RIGHT (0)
#define DISABLE_RIGHT (1)

#define LEDC_TIMER_RIGHT LEDC_TIMER_3
#define LEDC_MODE_RIGHT LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO_RIGHT (CONFIG_GPIO_MOTOR_RIGHT)
#define LEDC_CHANNEL_RIGHT LEDC_CHANNEL_3
#define LEDC_DUTY_RES_RIGHT LEDC_TIMER_13_BIT
#define LEDC_DUTY_RIGHT (4095)

static const char *TAG_MOTOR_RIGHT = "MOTOR RIGHT";

static xQueueHandle theta_right = NULL;
uint8_t end_sensor_right_check = 0;
double theta_3_send = 0;

void init_motor_right(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_RIGHT_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_RIGHT_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_RIGHT_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_RIGHT_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_RIGHT);

    theta_right = xQueueCreate(1, sizeof(double));
}

void pwm_right(uint8_t frequency_right)
{

    ledc_timer_config_t ledc_timer_right = {
        .speed_mode = LEDC_MODE_RIGHT,
        .timer_num = LEDC_TIMER_RIGHT,
        .duty_resolution = LEDC_DUTY_RES_RIGHT,
        .freq_hz = frequency_right,
        .clk_cfg = LEDC_AUTO_CLK};

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_right));

    ledc_channel_config_t ledc_channel_right = {
        .speed_mode = LEDC_MODE_RIGHT,
        .channel = LEDC_CHANNEL_RIGHT,
        .timer_sel = LEDC_TIMER_RIGHT,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO_RIGHT,
        .duty = 0,
        .hpoint = 0};

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_right));

    gpio_set_level(CONFIG_GPIO_MOTOR_RIGHT_ENABLE, ENABLE_RIGHT);
}

static void task_motor_right(void *arg)
{

    init_motor_right();
    uint8_t task_on_right = 0;

    double theta_right_value_new;
    double theta_right_value_old;

    double theta_right_value;
    int64_t start_timer_motor_right = 0;
    int64_t current_timer_motor_right = 0;
    double end_motor = 0;
    uint8_t start_count_motor_right = 0;

    while (1)
    {
        if (xQueueReceiveFromISR(theta_right, &theta_right_value, 100))
        {
            ESP_LOGI(TAG_MOTOR_RIGHT, "%lf...", theta_right_value);
            start_count_motor_right = 0;

            if (task_on_right == 1)
            {
                theta_right_value_new = theta_right_value;
                theta_right_value = get_new_theta(theta_right_value, theta_right_value_old, HORARIO_RIGHT, ANTI_HORARIO_RIGHT, CONFIG_GPIO_MOTOR_RIGHT_DIRECAO);

                theta_right_value_old = theta_right_value_new;
                end_sensor_right_check = 1;

                if (theta_right_value != 0)
                {
                    pwm_right(FREQUENCY_MAX);
                    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE_RIGHT, LEDC_CHANNEL_RIGHT, LEDC_DUTY_RIGHT));
                }

                ESP_LOGI(TAG_MOTOR_RIGHT, "theta original %f", theta_right_value_new);
                ESP_LOGI(TAG_MOTOR_RIGHT, "theta novo %f", theta_right_value);
                ESP_LOGI(TAG_MOTOR_RIGHT, "theta old %f", theta_right_value_old);
            }
            else
            {
                theta_right_value_old = theta_right_value;
                gpio_set_level(CONFIG_GPIO_MOTOR_RIGHT_DIRECAO, ANTI_HORARIO_RIGHT);
                pwm_right(FREQUENCY_MAX);
                ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE_RIGHT, LEDC_CHANNEL_RIGHT, LEDC_DUTY_RIGHT));
                task_on_right = 1;
            }
        }

        if (end_sensor_right_check == 1)
        {
            start_count_motor_right = 1;
            end_sensor_right_check = 0;
            start_timer_motor_right = esp_timer_get_time();
            end_motor = get_end_time(theta_right_value, FREQUENCY_MAX, 1, 1);
        }

        if (start_count_motor_right == 1)
        {
            current_timer_motor_right = esp_timer_get_time();
            theta_3_send = generate_values_of_theta(current_timer_motor_right);
        }

        if (start_count_motor_right == 1 && ((current_timer_motor_right - start_timer_motor_right) >= end_motor))
        {
            ledc_stop(LEDC_MODE_RIGHT, LEDC_CHANNEL_RIGHT, 0);
            gpio_set_level(CONFIG_GPIO_MOTOR_RIGHT_ENABLE, DISABLE_RIGHT);
            start_count_motor_right = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#endif