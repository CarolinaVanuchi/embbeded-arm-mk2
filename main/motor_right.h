#ifndef _MOTOR_RIGHT_
#define _MOTOR_RIGHT_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define SERVO_MIN_PULSEWIDTH_US (1000) // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US (2000) // Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE (1760)        // Maximum angle in degree upto which servo can rotate

static xQueueHandle theta_right = NULL;

static inline uint32_t convert_servo_angle_to_duty_us_right(int angle)
{
    return (angle + SERVO_MAX_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (2 * SERVO_MAX_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

void init_motor_right(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_DIREITO_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_DIREITO_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_DIREITO_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_DIREITO_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_DIREITO);

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, CONFIG_GPIO_MOTOR_DIREITO);

    mcpwm_config_t pwm_config = {
        .frequency = 500,
        .cmpr_a = 0,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

    gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO_DIRECAO, 0);
    gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO_ENABLE, 0);

    theta_right = xQueueCreate(1, sizeof(double));
}

static void task_motor_right(void *arg)
{
    init_motor_right();
    double theta_right_value;
    int angle = -SERVO_MAX_DEGREE;

    while (1)
    {

        if (xQueueReceiveFromISR(theta_right, &theta_right_value, 100))
        {
            ESP_LOGI("right angles", "%lf...", theta_right_value);
        }

        ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, convert_servo_angle_to_duty_us_right(angle)));
        vTaskDelay(pdMS_TO_TICKS(100));
        angle++;
    }
}

#endif