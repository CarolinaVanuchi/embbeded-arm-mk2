#ifndef _FINAL_ACTUATOR_
#define _FINAL_ACTUATOR_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "driver/mcpwm.h"
#include <sys/param.h>
#include "sdkconfig.h"
#include "led.h"

#define SERVO_MIN_PULSEWIDTH_US (1000)
#define SERVO_MAX_PULSEWIDTH_US (2000)

#define SERVO_MAX_DEGREE (90)
#define SERVO_MIN_DEGREE (0)
static const char *TAG_GARRA = "MOTOR GARRA";

bool finish_end = false;
bool finish_left = false;
bool finish_right = false;

static inline uint32_t convert_servo_angle_to_duty_us(int angle)
{
    return (angle + SERVO_MAX_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (2 * SERVO_MAX_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

void init_final_actuator()
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, CONFIG_GPIO_MICRO_SERVO);

    mcpwm_config_t pwm_config = {
        .frequency = 50,
        .cmpr_a = 0,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
}

void open_final_actuator()
{
    ESP_LOGI(TAG_GARRA, "OPEN");
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, convert_servo_angle_to_duty_us(180));
}

void close_final_actuator()
{
    while (1)
    {
        if (finish_end && finish_left && finish_right)
        {
            ESP_LOGI(TAG_GARRA, "CLOSE");
            off_led();
            finish_end = false;
            finish_left = false;
            finish_right = false;
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, convert_servo_angle_to_duty_us(45));
        }
        esp_task_wdt_reset();
    }
}

#endif