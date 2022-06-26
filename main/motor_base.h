#ifndef _MOTOR_BASE_
#define _MOTOR_BASE_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
// #include "freertos/timers.h"
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
#define FREQUENCY_MIN_BASE (100)
#define RESOLUCAO (30)

#define WITH_RELOAD 1
#define TIMER_INTR_US 7
#define TIMER_DIVIDER 16
#define TIMER_TICKS (TIMER_BASE_CLK / TIMER_DIVIDER)
#define SEC_TO_MICRO_SEC(x) ((x) / 1000 / 1000)
#define ALARM_VAL_US SEC_TO_MICRO_SEC(TIMER_INTR_US *TIMER_TICKS)

#define TIMER_GROUP_BASE (TIMER_GROUP_1)
#define TIMER_BASE (TIMER_0)

static const char *TAG_MOTOR_BASE = "MOTOR BASE";

static xQueueHandle theta_base = NULL;
wave_t *wave_g = NULL;
// TimerHandle_t xTimerBase;

void init_motor_base(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE, GPIO_MODE_OUTPUT);

    theta_base = xQueueCreate(1, sizeof(double));
}

static bool IRAM_ATTR on_timer_alarm_cb(void *user_data)
{
    if (wave_g != NULL)
    {
        gpio_set_level(CONFIG_GPIO_MOTOR_BASE_DIRECAO, ANTI_HORARIO_BASE);
        gpio_set_level(CONFIG_GPIO_MOTOR_BASE, waveRead(wave_g));
    }

    return pdFALSE;
}

// static void vTimerCallback(TimerHandle_t xTimer)
// {
//     gpio_set_level(CONFIG_GPIO_MOTOR_BASE, waveRead(wave_g));
// }

void init_timer_base(void)
{

    // xTimerBase = xTimerCreate(
    //     "xTimer Base",
    //     pdMS_TO_TICKS(0.1),
    //     pdTRUE,
    //     (void *)0,
    //     vTimerCallback);

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

static void task_motor_base(void *arg)
{
    init_motor_base();

    double theta_base_value;
    while (1)
    {
        if(xQueueReceive(theta_base, &theta_base_value, 10))
        {
            // timer_pause(TIMER_GROUP_BASE, TIMER_BASE);
            ESP_LOGI(TAG_MOTOR_BASE, "%lf...", theta_base_value);
            if (wave_g != NULL)
            {
                waveDelete(wave_g);
            }

            wave_g = waveGenStepMotorSineAcceleration(get_step(theta_base_value, 8, 1), FREQUENCY_MIN_BASE, FREQUENCY_MAX_BASE, RESOLUCAO);

            // for(size_t i = 0; i < wave_g->points->size; i++)
            //     ESP_LOGI("P", "%i", waveRead(wave_g));
            
            // waveReadReset(wave_g);

            // xTimerStart(xTimerBase, pdMS_TO_TICKS(10));
            init_timer_base();
            timer_set_alarm_value(TIMER_GROUP_BASE, TIMER_BASE, (uint64_t)ceil(wave_g->period * (1000000ULL)));
            uint32_t aa = ceil(wave_g->period * (1000000ULL));
            ESP_LOGI("t", "%f", wave_g->period);
            ESP_LOGI("P", "%d", aa);
            timer_start(TIMER_GROUP_BASE, TIMER_BASE);
        }
        esp_task_wdt_reset();
    }
}

#endif