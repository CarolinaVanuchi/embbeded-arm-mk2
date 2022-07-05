#ifndef _MOTOR_BASE_
#define _MOTOR_BASE_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include <math.h>
#include "wavegen.h"
#include "esp_log.h"
#include "driver/timer.h"
#include "generic_motor.h"
#include "inttypes.h"

#define HORARIO_BASE (1)
#define ANTI_HORARIO_BASE (0)

#define ENABLE_BASE (0)
#define DISABLE_BASE (1)
#define FREQUENCY_MAX_BASE (250)
#define FREQUENCY_MIN_BASE (220)
#define FREQUENCY_BASE (200)
#define RESOLUCAO (10)

#define TIMER_GROUP_BASE (TIMER_GROUP_1)
#define TIMER_BASE (TIMER_0)

#define LEDC_TIMER_BASE LEDC_TIMER_0
#define LEDC_MODE_BASE LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO_BASE (CONFIG_GPIO_MOTOR_BASE)
#define LEDC_CHANNEL_BASE LEDC_CHANNEL_0
#define LEDC_DUTY_RES_BASE LEDC_TIMER_13_BIT
#define LEDC_DUTY_BASE (4095)

static const char *TAG_MOTOR_BASE = "MOTOR BASE";

static xQueueHandle theta_base = NULL;
wave_t *wave_g = NULL;

uint8_t end_sensor_base_check = 0;

/**
 * @brief Inicializa os pinos dos sensores da base
 *
 */
void init_end_base(void)
{
    gpio_config_t end_base_sensor = {.mode = GPIO_MODE_INPUT, .pin_bit_mask = (1ULL << CONFIG_GPIO_END_BASE)};
    gpio_config(&end_base_sensor);
}

void init_motor_base(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE, GPIO_MODE_OUTPUT);

    gpio_set_level(CONFIG_GPIO_MOTOR_BASE_DIRECAO, ANTI_HORARIO_BASE);
    gpio_set_level(CONFIG_GPIO_MOTOR_BASE_ENABLE, ENABLE_BASE);
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

    ledc_timer_config(&ledc_timer_base);

    ledc_channel_config_t ledc_channel_base = {
        .speed_mode = LEDC_MODE_BASE,
        .channel = LEDC_CHANNEL_BASE,
        .timer_sel = LEDC_TIMER_BASE,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO_BASE,
        .duty = 0,
        .hpoint = 0};

    ledc_channel_config(&ledc_channel_base);
}

static bool IRAM_ATTR on_timer_alarm_cb(void *user_data)
{
    if (wave_g != NULL)
    {
        gpio_set_level(CONFIG_GPIO_MOTOR_BASE, waveRead(wave_g));
    }

    return pdFALSE;
}

void init_timer_base(void)
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

    ret = timer_init(TIMER_GROUP_BASE, TIMER_BASE, &config);
    ESP_ERROR_CHECK(ret);
    timer_set_counter_value(TIMER_GROUP_BASE, TIMER_BASE, 0);
    ret = timer_enable_intr(TIMER_GROUP_BASE, TIMER_BASE);
    ESP_ERROR_CHECK(ret);
    timer_isr_callback_add(TIMER_GROUP_BASE, TIMER_BASE, on_timer_alarm_cb, NULL, 0);
}

void init_move_base(double theta_base)
{
    wave_g = waveGenStepMotorSineAcceleration(get_step(theta_base, 8, 5.5, 1), FREQUENCY_MIN_BASE, FREQUENCY_MAX_BASE, RESOLUCAO);
    // wave_g = waveGenStepMotorSineAcceleration(get_step(theta_base, 8, 1, 1), FREQUENCY_MIN_BASE, FREQUENCY_MAX_BASE, RESOLUCAO);
    ESP_LOGI(TAG_MOTOR_BASE, "%i", wave_g->points->size);
    timer_set_alarm_value(TIMER_GROUP_BASE, TIMER_BASE, (uint64_t)ceil(wave_g->period * (1000000ULL)));
    timer_start(TIMER_GROUP_BASE, TIMER_BASE);
}

static void task_motor_base(void *arg)
{
    init_end_base();
    init_motor_base();

    double theta_base_value;
    double theta_base_value_new;
    double theta_base_value_old = 0;

    bool start_now = true;
    bool start_run = false;
    bool not_first = false;
    bool check_base = false;

    while (1)
    {

        if (xQueueReceive(theta_base, &theta_base_value, 10))
        {
            ESP_LOGI(TAG_MOTOR_BASE, "%lf...", theta_base_value);

            if (!start_now)
                start_run = true;

            if (start_now)
            {
                theta_base_value_old = theta_base_value;
                pwm_base(FREQUENCY_BASE);
                ledc_set_duty(LEDC_MODE_BASE, LEDC_CHANNEL_BASE, LEDC_DUTY_BASE);
            }
        }

        if ((gpio_get_level(CONFIG_GPIO_END_BASE) == 0) && !start_run && !check_base)
        {
            start_now = false;

            ledc_stop(LEDC_MODE_BASE, LEDC_CHANNEL_BASE, 0);

            start_run = true;

            gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE);
            gpio_set_direction(CONFIG_GPIO_MOTOR_BASE, GPIO_MODE_OUTPUT);
            gpio_set_level(CONFIG_GPIO_MOTOR_BASE_DIRECAO, HORARIO_BASE);

            init_timer_base();
            check_base = true;
        }

        if (start_run)
        {
            if (not_first)
            {
                theta_base_value_new = theta_base_value;
                theta_base_value = get_new_theta(theta_base_value, theta_base_value_old, HORARIO_BASE, ANTI_HORARIO_BASE, CONFIG_GPIO_MOTOR_BASE_DIRECAO);
                theta_base_value_old = theta_base_value_new;
            }

            not_first = true;

            if (wave_g != NULL)
            {
                waveDelete(wave_g);
                wave_g = NULL;
            }

            if (theta_base_value > 0)
            {
                gpio_set_level(CONFIG_GPIO_MOTOR_BASE_ENABLE, ENABLE_BASE);
                init_move_base(theta_base_value);
            }
            else
            {
                gpio_set_level(CONFIG_GPIO_MOTOR_BASE_ENABLE, DISABLE_BASE);
            }

            start_run = false;
        }

        esp_task_wdt_reset();
    }
}

#endif