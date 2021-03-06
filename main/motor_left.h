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
#define FREQUENCY_MAX_LEFT (150)
#define FREQUENCY_MIN_LEFT (100)
#define FREQUENCY_LEFT (100)
#define RESOLUCAO_LEFT (10)

#define ENABLE_LEFT (0)
#define DISABLE_LEFT (1)

#define LEDC_TIMER_LEFT LEDC_TIMER_1
#define LEDC_MODE_LEFT LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO_LEFT (CONFIG_GPIO_MOTOR_LEFT)
#define LEDC_CHANNEL_LEFT LEDC_CHANNEL_1
#define LEDC_DUTY_RES_LEFT LEDC_TIMER_13_BIT
#define LEDC_DUTY_LEFT (4095)

#define TIMER_GROUP_LEFT (TIMER_GROUP_1)

#define TIMER_LEFT (TIMER_1)

static const char *TAG_MOTOR_LEFT = "MOTOR LEFT";
wave_t *wave_g_left = NULL;

static xQueueHandle theta_left = NULL;
bool end_sensor_left_check = false;
double theta_2_send = 0;

int total_points_left = 0;
int current_point_left = 0;
bool running_left = false;

void init_motor_left(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_LEFT_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_LEFT_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_LEFT_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_LEFT_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_set_direction(CONFIG_GPIO_MOTOR_LEFT, GPIO_MODE_OUTPUT);
    gpio_set_level(CONFIG_GPIO_MOTOR_LEFT_DIRECAO, ANTI_HORARIO_LEFT);
    gpio_set_level(CONFIG_GPIO_MOTOR_LEFT_ENABLE, ENABLE_LEFT);

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

    ledc_timer_config(&ledc_timer_left);

    ledc_channel_config_t ledc_channel_left = {
        .speed_mode = LEDC_MODE_LEFT,
        .channel = LEDC_CHANNEL_LEFT,
        .timer_sel = LEDC_TIMER_LEFT,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO_LEFT,
        .duty = 0,
        .hpoint = 0};

    ledc_channel_config(&ledc_channel_left);

    gpio_set_level(CONFIG_GPIO_MOTOR_LEFT_ENABLE, ENABLE_LEFT);
}

static bool IRAM_ATTR on_timer_alarm_cb_left(void *user_data)
{
    if (wave_g_left != NULL)
    {
        current_point_left = wave_g_left->index;
        gpio_set_level(CONFIG_GPIO_MOTOR_LEFT, waveRead(wave_g_left));
        if ((current_point_left >= total_points_left) && (running_left))
        {
            running_left = false;
            finish_left = true;
        }
    }

    return pdFALSE;
}

void init_timer_left(void)
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

    ret = timer_init(TIMER_GROUP_LEFT, TIMER_LEFT, &config);
    ESP_ERROR_CHECK(ret);
    timer_set_counter_value(TIMER_GROUP_LEFT, TIMER_LEFT, 0);
    ret = timer_enable_intr(TIMER_GROUP_LEFT, TIMER_LEFT);
    ESP_ERROR_CHECK(ret);
    timer_isr_callback_add(TIMER_GROUP_LEFT, TIMER_LEFT, on_timer_alarm_cb_left, NULL, 0);
}

void init_move_left(double theta_left_v)
{
    wave_g_left = waveGenStepMotorSineAcceleration(get_step(theta_left_v, 2.5, 4.50, 4), FREQUENCY_MIN_LEFT, FREQUENCY_MAX_LEFT, RESOLUCAO_LEFT);
    // wave_g_left = waveGenStepMotorSineAcceleration(get_step(theta_left_v, 1, 4.50, 4), FREQUENCY_MIN_LEFT, FREQUENCY_MAX_LEFT, RESOLUCAO_LEFT);
    total_points_left = wave_g_left->points->size;
    ESP_LOGI(TAG_MOTOR_LEFT, "%i", wave_g_left->points->size);
    timer_set_alarm_value(TIMER_GROUP_LEFT, TIMER_LEFT, (uint64_t)ceil(wave_g_left->period * (1000000ULL)));
    timer_start(TIMER_GROUP_LEFT, TIMER_LEFT);
}

static void task_motor_left(void *arg)
{

    init_motor_left();

    double theta_left_value;
    double theta_left_value_new;
    double theta_left_value_old = 0;

    bool start_now_left = true;
    bool start_run_left = false;
    bool not_first_left = false;

    bool check_extra = false;

    while (1)
    {
        if (xQueueReceive(theta_left, &theta_left_value, 10))
        {
            ESP_LOGI(TAG_MOTOR_LEFT, "%lf...", theta_left_value);
            running_left = true;
            if (!start_now_left)
                start_run_left = true;

            if (start_now_left)
            {
                theta_left_value_old = theta_left_value;
                pwm_left(FREQUENCY_LEFT);
                ledc_set_duty(LEDC_MODE_LEFT, LEDC_CHANNEL_LEFT, LEDC_DUTY_LEFT);
            }
        }

        if (end_sensor_left_check && !start_run_left && !check_extra)
        {
            check_extra = true;
            start_now_left = false;
            end_sensor_left_check = false;
            ledc_stop(LEDC_MODE_LEFT, LEDC_CHANNEL_LEFT, 0);

            start_run_left = true;

            gpio_reset_pin(CONFIG_GPIO_MOTOR_LEFT);
            gpio_set_direction(CONFIG_GPIO_MOTOR_LEFT, GPIO_MODE_OUTPUT);
            gpio_set_level(CONFIG_GPIO_MOTOR_LEFT_DIRECAO, HORARIO_LEFT);

            init_timer_left();
        }

        if (start_run_left)
        {
            if (not_first_left)
            {
                theta_left_value_new = theta_left_value;
                theta_left_value = get_new_theta(theta_left_value, theta_left_value_old, HORARIO_LEFT, ANTI_HORARIO_LEFT, CONFIG_GPIO_MOTOR_LEFT_DIRECAO);
                theta_left_value_old = theta_left_value_new;
            }

            not_first_left = true;

            if (wave_g_left != NULL)
            {
                waveDelete(wave_g_left);
                wave_g_left = NULL;
            }

            if (theta_left_value > 0)
            {
                ESP_LOGI(TAG_MOTOR_LEFT, "NOVO THETA LEFT: %f", theta_left_value);
                ESP_LOGI(TAG_MOTOR_LEFT, "OLD THETA LEFT: %f", theta_left_value_old);

                gpio_set_level(CONFIG_GPIO_MOTOR_LEFT_ENABLE, ENABLE_LEFT);
                init_move_left(theta_left_value);
            }
            else
            {
                gpio_set_level(CONFIG_GPIO_MOTOR_LEFT_ENABLE, DISABLE_LEFT);
                finish_left = true;
            }

            start_run_left = false;
        }

        esp_task_wdt_reset();
    }
}

#endif