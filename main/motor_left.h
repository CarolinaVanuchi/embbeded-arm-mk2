#ifndef _MOTOR_LEFT_
#define _MOTOR_LEFT_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "generic_motor.h"
#include "uart.h"

#define HORARIO_LEFT (0)
#define ANTI_HORARIO_LEFT (1)
#define FREQUENCY_MAX_LEFT (250)

#define ENABLE_LEFT (0)
#define DISABLE_LEFT (1)

#define LEDC_TIMER_LEFT LEDC_TIMER_1
#define LEDC_MODE_LEFT LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO_LEFT (CONFIG_GPIO_MOTOR_LEFT) 
#define LEDC_CHANNEL_LEFT LEDC_CHANNEL_1
#define LEDC_DUTY_RES_LEFT LEDC_TIMER_13_BIT 
#define LEDC_DUTY_LEFT (4095) 

static const char *TAG_MOTOR_LEFT = "MOTOR LEFT";

static xQueueHandle theta_left = NULL;
uint8_t end_sensor_left_check = 0;
double theta_2_send = 0;

void init_motor_left(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_LEFT_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_LEFT_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_LEFT_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_LEFT_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_LEFT);

    theta_left = xQueueCreate(1, sizeof(double));
}

void pwm_left(uint8_t frequency_left)
{

    ledc_timer_config_t ledc_timer_left = {
        .speed_mode = LEDC_MODE_LEFT,
        .timer_num = LEDC_TIMER_LEFT,
        .duty_resolution = LEDC_DUTY_RES_LEFT,
        .freq_hz = frequency_left,
        .clk_cfg = LEDC_AUTO_CLK};

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_left));

    ledc_channel_config_t ledc_channel_left = {
        .speed_mode = LEDC_MODE_LEFT,
        .channel = LEDC_CHANNEL_LEFT,
        .timer_sel = LEDC_TIMER_LEFT,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO_LEFT,
        .duty = 0,
        .hpoint = 0};

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_left));

    gpio_set_level(CONFIG_GPIO_MOTOR_LEFT_ENABLE, ENABLE_LEFT);
}

static void task_motor_left(void *arg)
{

    init_motor_left();
    uint8_t task_on_left = 0;

    double theta_left_value_new;
    double theta_left_value_old;

    double theta_left_value;
    int64_t start_timer_motor_left = 0;
    int64_t current_timer_motor_left = 0;
    double end_motor = 0;
    uint8_t start_count_motor_left = 0;

    while (1)
    {
        if (xQueueReceive(theta_left, &theta_left_value, 10))
        {
            ESP_LOGI(TAG_MOTOR_LEFT, "%lf...", theta_left_value);
            start_count_motor_left = 0;

            if (task_on_left == 1)
            {
                theta_left_value_new = theta_left_value;
                theta_left_value = get_new_theta(theta_left_value, theta_left_value_old, HORARIO_LEFT, ANTI_HORARIO_LEFT, CONFIG_GPIO_MOTOR_LEFT_DIRECAO);

                theta_left_value_old = theta_left_value_new;
                end_sensor_left_check = 1;

                if (theta_left_value != 0)
                {
                    pwm_left(FREQUENCY_MAX_LEFT);
                    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE_LEFT, LEDC_CHANNEL_LEFT, LEDC_DUTY_LEFT));
                }

                ESP_LOGI(TAG_MOTOR_LEFT, "theta original %f", theta_left_value_new);
                ESP_LOGI(TAG_MOTOR_LEFT, "theta novo %f", theta_left_value);
                ESP_LOGI(TAG_MOTOR_LEFT, "theta old %f", theta_left_value_old);
            }
            else
            {
                theta_left_value_old = theta_left_value;
                gpio_set_level(CONFIG_GPIO_MOTOR_LEFT_DIRECAO, ANTI_HORARIO_LEFT);
                pwm_left(FREQUENCY_MAX_LEFT);
                ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE_LEFT, LEDC_CHANNEL_LEFT, LEDC_DUTY_LEFT));
                task_on_left = 1;
            }
        }

        if (end_sensor_left_check == 1)
        {
            start_count_motor_left = 1;
            end_sensor_left_check = 0;
            start_timer_motor_left = esp_timer_get_time();
            end_motor = get_end_time(theta_left_value, FREQUENCY_MAX_LEFT, 4, 10.8);
        }

        if (start_count_motor_left == 1)
        {
            current_timer_motor_left = esp_timer_get_time();
        }

        if (start_count_motor_left == 1 && ((current_timer_motor_left - start_timer_motor_left) >= end_motor))
        {
            ledc_stop(LEDC_MODE_LEFT, LEDC_CHANNEL_LEFT, 0);
            gpio_set_level(CONFIG_GPIO_MOTOR_LEFT_ENABLE, DISABLE_LEFT);
            start_count_motor_left = 0;
        }
       esp_task_wdt_reset();
    }
}

#endif