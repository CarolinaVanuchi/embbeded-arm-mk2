#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include <sys/param.h>

#ifndef _FINAL_ACTUATOR_
#define _FINAL_ACTUATOR_

#define SERVO_MIN_PULSEWIDTH_US (1000) // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US (2000) // Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE (90)          // Maximum angle in degree upto which servo can rotate

static inline uint32_t convert_servo_angle_to_duty_us(int angle)
{
    return (angle + SERVO_MAX_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (2 * SERVO_MAX_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

void set_config_final_actuator()
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, CONFIG_GPIO_MICRO_SERVO); // To drive a RC servo, one MCPWM generator is enough

    mcpwm_config_t pwm_config = {
        .frequency = 50, // frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        .cmpr_a = 0,     // duty cycle of PWMxA = 0
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
}

void open_final_actuator()
{
    for (int angle = SERVO_MAX_DEGREE; angle > SERVO_MAX_DEGREE; angle--)
    {
        ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, convert_servo_angle_to_duty_us(angle)));
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void close_final_actuator()
{
    for (int angle = -SERVO_MAX_DEGREE; angle < SERVO_MAX_DEGREE; angle++)
    {
        ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, convert_servo_angle_to_duty_us(angle)));
        vTaskDelay(pdMS_TO_TICKS(1)); // Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation under 5V power supply
    }
}

#endif