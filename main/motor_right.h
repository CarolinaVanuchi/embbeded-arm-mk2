#ifndef _MOTOR_RIGHT_
#define _MOTOR_RIGHT_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "generic_motor.h"
#include "uart.h"

#define HORARIO_RIGHT (0)
#define ANTI_HORARIO_RIGHT (1)
#define FREQUENCY_MAX_RIGHT (250)
#define FREQUENCY_MIN_RIGHT (100)
#define RESOLUCAO_RIGHT (30)

#define ENABLE_RIGHT (0)
#define DISABLE_RIGHT (1)

#define LEDC_TIMER_RIGHT LEDC_TIMER_2
#define LEDC_MODE_RIGHT LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO_RIGHT (CONFIG_GPIO_MOTOR_RIGHT)
#define LEDC_CHANNEL_RIGHT LEDC_CHANNEL_2
#define LEDC_DUTY_RES_RIGHT LEDC_TIMER_13_BIT
#define LEDC_DUTY_RIGHT (4095)

#define TIMER_GROUP_RIGHT (TIMER_GROUP_0)
#define TIMER_RIGHT (TIMER_1)

static const char *TAG_MOTOR_RIGHT = "MOTOR RIGHT";
wave_t *wave_g_right = NULL;

static xQueueHandle theta_right = NULL;
bool end_sensor_right_check = false;
double theta_3_send = 0;

void init_motor_right(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_RIGHT_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_RIGHT_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_RIGHT_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_RIGHT_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_set_direction(CONFIG_GPIO_MOTOR_RIGHT, GPIO_MODE_OUTPUT);
    gpio_set_level(CONFIG_GPIO_MOTOR_RIGHT_DIRECAO, ANTI_HORARIO_RIGHT);
    gpio_set_level(CONFIG_GPIO_MOTOR_RIGHT_ENABLE, ENABLE_RIGHT);

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

    ledc_timer_config(&ledc_timer_right);

    ledc_channel_config_t ledc_channel_right = {
        .speed_mode = LEDC_MODE_RIGHT,
        .channel = LEDC_CHANNEL_RIGHT,
        .timer_sel = LEDC_TIMER_RIGHT,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO_RIGHT,
        .duty = 0,
        .hpoint = 0};

    ledc_channel_config(&ledc_channel_right);

    gpio_set_level(CONFIG_GPIO_MOTOR_RIGHT_ENABLE, ENABLE_RIGHT);
}

static bool IRAM_ATTR on_timer_alarm_cb_right(void *user_data)
{
    if (wave_g_right != NULL)
    {
        gpio_set_level(CONFIG_GPIO_MOTOR_RIGHT, waveRead(wave_g_right));
    }

    return pdFALSE;
}

void init_timer_right(void)
{

    esp_err_t ret;

    timer_config_t config = {
        .divider = 80,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .intr_type = TIMER_INTR_LEVEL,
        .auto_reload = TIMER_AUTORELOAD_EN,
    };

    ret = timer_init(TIMER_GROUP_RIGHT, TIMER_RIGHT, &config);
    ESP_ERROR_CHECK(ret);
    timer_set_counter_value(TIMER_GROUP_RIGHT, TIMER_RIGHT, 0);
    ret = timer_enable_intr(TIMER_GROUP_RIGHT, TIMER_RIGHT);
    ESP_ERROR_CHECK(ret);
    timer_isr_callback_add(TIMER_GROUP_RIGHT, TIMER_RIGHT, on_timer_alarm_cb_right, NULL, 0);
}

void init_move_right(double theta_right_v)
{
    wave_g_right = waveGenStepMotorSineAcceleration(get_step(theta_right_v, 4, 1, 1), FREQUENCY_MIN_RIGHT, FREQUENCY_MAX_RIGHT, RESOLUCAO_RIGHT);

    timer_set_alarm_value(TIMER_GROUP_RIGHT, TIMER_RIGHT, (uint64_t)ceil(wave_g_right->period * (1000000ULL)));
    timer_start(TIMER_GROUP_RIGHT, TIMER_RIGHT);
}

static void task_motor_right(void *arg)
{

    init_motor_right();

    double theta_right_value;
    double theta_right_value_new;
    double theta_right_value_old = 0;

    bool start_now_right = true;
    bool start_run_right = false;
    bool not_first_right = false;
    bool check_extra_right = false;

    while (1)
    {
        if (xQueueReceive(theta_right, &theta_right_value, 10))
        {
            ESP_LOGI(TAG_MOTOR_RIGHT, "A");

            if (!start_now_right)
                start_run_right = true;

            if (start_now_right)
            {
                theta_right_value_old = theta_right_value;
                pwm_right(FREQUENCY_MIN_RIGHT);
                ledc_set_duty(LEDC_MODE_RIGHT, LEDC_CHANNEL_RIGHT, LEDC_DUTY_RIGHT);
            }
        }

        if (end_sensor_right_check && !start_run_right && !check_extra_right)
        {
            ESP_LOGI(TAG_MOTOR_RIGHT, "B");
            check_extra_right = true;
            start_now_right = false;
            end_sensor_right_check = false;
            ledc_stop(LEDC_MODE_RIGHT, LEDC_CHANNEL_RIGHT, 0);

            start_run_right = true;

            gpio_reset_pin(CONFIG_GPIO_MOTOR_RIGHT);
            gpio_set_direction(CONFIG_GPIO_MOTOR_RIGHT, GPIO_MODE_OUTPUT);
            gpio_set_level(CONFIG_GPIO_MOTOR_RIGHT_DIRECAO, HORARIO_RIGHT);

            init_timer_right();
        }

        if (start_run_right)
        {
            ESP_LOGI(TAG_MOTOR_RIGHT, "C");
            if (not_first_right)
            {
                ESP_LOGI(TAG_MOTOR_RIGHT, "D");
                theta_right_value_new = theta_right_value;
                theta_right_value = get_new_theta(theta_right_value, theta_right_value_old, HORARIO_RIGHT, ANTI_HORARIO_RIGHT, CONFIG_GPIO_MOTOR_RIGHT_DIRECAO);
                theta_right_value_old = theta_right_value_new;
            }

            not_first_right = true;

            if (wave_g_right != NULL)
            {
                ESP_LOGI(TAG_MOTOR_RIGHT, "E");
                waveDelete(wave_g_right);
                wave_g_right = NULL;
            }

            if (theta_right_value > 0)
            {
                ESP_LOGI(TAG_MOTOR_RIGHT, "NOVO THETA RIGHT: %f", theta_right_value);
                ESP_LOGI(TAG_MOTOR_RIGHT, "OLD THETA RIGHT: %f", theta_right_value_old);

                gpio_set_level(CONFIG_GPIO_MOTOR_RIGHT_ENABLE, ENABLE_RIGHT);
                init_move_right(theta_right_value);
            }
            else
            {
                ESP_LOGI(TAG_MOTOR_RIGHT, "F");
                gpio_set_level(CONFIG_GPIO_MOTOR_RIGHT_ENABLE, DISABLE_RIGHT);
            }

            start_run_right = false;
        }

        esp_task_wdt_reset();
    }
}

#endif