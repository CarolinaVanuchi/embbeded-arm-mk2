#ifndef _SENSOR_
#define _SENSOR_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "motor_base.h"

#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle gpio_end_motor_esquerdo = NULL; // M2
static xQueueHandle gpio_end_motor_direito = NULL;  // M3

static void IRAM_ATTR gpio_isr_handler_esquerdo(void *arg)
{
    uint32_t gpio_sensor = (uint32_t)arg;
    xQueueSendFromISR(gpio_end_motor_esquerdo, &gpio_sensor, NULL);
}

static void IRAM_ATTR gpio_isr_handler_direito(void *arg)
{
    uint32_t gpio_sensor = (uint32_t)arg;
    xQueueSendFromISR(gpio_end_motor_direito, &gpio_sensor, NULL);
}

void init_end_left(void)
{
    gpio_reset_pin(CONFIG_GPIO_END_MOTOR_LEFT);
    gpio_set_direction(CONFIG_GPIO_END_MOTOR_LEFT, GPIO_MODE_INPUT);

    gpio_config_t config_end_esquerdo = {
        .intr_type = GPIO_PIN_INTR_NEGEDGE,
        .pin_bit_mask = (1ULL << CONFIG_GPIO_END_MOTOR_LEFT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
    };

    gpio_config(&config_end_esquerdo);

    gpio_end_motor_esquerdo = xQueueCreate(1, sizeof(uint32_t));
    gpio_isr_handler_add(CONFIG_GPIO_END_MOTOR_LEFT, gpio_isr_handler_esquerdo, (void *)CONFIG_GPIO_END_MOTOR_LEFT);
}

void init_end_right(void)
{
    gpio_reset_pin(CONFIG_GPIO_END_MOTOR_RIGHT);
    gpio_set_direction(CONFIG_GPIO_END_MOTOR_RIGHT, GPIO_MODE_INPUT);

    gpio_config_t config_end_direito = {
        .intr_type = GPIO_PIN_INTR_POSEDGE,
        .pin_bit_mask = (1ULL << CONFIG_GPIO_END_MOTOR_RIGHT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
    };

    gpio_config(&config_end_direito);

    gpio_end_motor_direito = xQueueCreate(1, sizeof(uint32_t));
    gpio_isr_handler_add(CONFIG_GPIO_END_MOTOR_RIGHT, gpio_isr_handler_direito, (void *)CONFIG_GPIO_END_MOTOR_RIGHT);
}

static void init_sensor(void)
{
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    init_end_left();
    init_end_right();
}

static void task_sensor(void *arg)
{
    
    uint32_t gpio_sensor_left;
    uint32_t gpio_sensor_right;

    while (1)
    {

        if (xQueueReceive(gpio_end_motor_esquerdo, &gpio_sensor_left, 10))
        {
            ESP_LOGI("ISR", "Fim de curso motor 2...");
            end_sensor_left_check = true;
        }

        if (xQueueReceive(gpio_end_motor_direito, &gpio_sensor_right, 10))
        {
            ESP_LOGI("ISR", "Fim de curso motor 3...");
            gpio_set_level(CONFIG_GPIO_MOTOR_RIGHT_DIRECAO, HORARIO_RIGHT);
            end_sensor_right_check = 1;
        }

        esp_task_wdt_reset();
    }
}

#endif
