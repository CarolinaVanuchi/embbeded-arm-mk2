#ifndef _ROBOT_
#define _ROBOT_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define ESP_INTR_FLAG_DEFAULT 0
typedef struct
{
    double theta_1;
    double theta_2;
    double theta_3;
} List_thetas;

static xQueueHandle gpio_end_motor_base = NULL;     // M1
static xQueueHandle gpio_end_motor_esquerdo = NULL; // M2
static xQueueHandle gpio_end_motor_direito = NULL;  // M3
static xQueueHandle thetas = NULL;

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

void init_motor_base(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_BASE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_BASE, GPIO_MODE_OUTPUT);
}

void init_motor_esquerdo(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_ESQUERDO_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_ESQUERDO_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_ESQUERDO_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_ESQUERDO_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_ESQUERDO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_ESQUERDO, GPIO_MODE_OUTPUT);
}

void init_motor_direito(void)
{
    gpio_reset_pin(CONFIG_GPIO_MOTOR_DIREITO_ENABLE);
    gpio_set_direction(CONFIG_GPIO_MOTOR_DIREITO_ENABLE, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_DIREITO_DIRECAO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_DIREITO_DIRECAO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(CONFIG_GPIO_MOTOR_DIREITO);
    gpio_set_direction(CONFIG_GPIO_MOTOR_DIREITO, GPIO_MODE_OUTPUT);
}

void init_end_left(void)
{
    gpio_reset_pin(CONFIG_GPIO_END_MOTOR_ESQUERDO);
    gpio_set_direction(CONFIG_GPIO_END_MOTOR_ESQUERDO, GPIO_MODE_INPUT);

    gpio_config_t config_end_esquerdo = {
        .intr_type = GPIO_PIN_INTR_NEGEDGE,
        .pin_bit_mask = (1ULL << CONFIG_GPIO_END_MOTOR_ESQUERDO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
    };

    gpio_config(&config_end_esquerdo);

    gpio_end_motor_esquerdo = xQueueCreate(1, sizeof(uint32_t));
    gpio_isr_handler_add(CONFIG_GPIO_END_MOTOR_ESQUERDO, gpio_isr_handler_esquerdo, (void *)CONFIG_GPIO_END_MOTOR_ESQUERDO);
}

void init_end_right(void)
{
    gpio_reset_pin(CONFIG_GPIO_END_MOTOR_DIREITO);
    gpio_set_direction(CONFIG_GPIO_END_MOTOR_DIREITO, GPIO_MODE_INPUT);

    gpio_config_t config_end_direito = {
        .intr_type = GPIO_PIN_INTR_POSEDGE,
        .pin_bit_mask = (1ULL << CONFIG_GPIO_END_MOTOR_DIREITO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
    };

    gpio_config(&config_end_direito);

    gpio_end_motor_direito = xQueueCreate(1, sizeof(uint32_t));
    gpio_isr_handler_add(CONFIG_GPIO_END_MOTOR_DIREITO, gpio_isr_handler_direito, (void *)CONFIG_GPIO_END_MOTOR_DIREITO);
}

static void init_robot(void)
{
    init_motor_base();
    init_motor_esquerdo();
    init_motor_direito();
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    thetas = xQueueCreate(3, sizeof(List_thetas));
    gpio_end_motor_base = xQueueCreate(1, sizeof(uint8_t));
    init_end_left();
    init_end_right();
}

void move_motores()
{
    gpio_set_level(CONFIG_GPIO_MOTOR_BASE, 1);
    gpio_set_level(CONFIG_GPIO_MOTOR_ESQUERDO, 1);
    gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO, 1);
    vTaskDelay(10 / portTICK_PERIOD_MS);

    gpio_set_level(CONFIG_GPIO_MOTOR_BASE, 0);
    gpio_set_level(CONFIG_GPIO_MOTOR_ESQUERDO, 0);
    gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

static void task_robot(void *arg)
{
    uint32_t gpio_sensor_base;
    uint32_t gpio_sensor_left;
    uint32_t gpio_sensor_right;
    List_thetas itens;

    gpio_set_level(CONFIG_GPIO_MOTOR_BASE_DIRECAO, 0);
    gpio_set_level(CONFIG_GPIO_MOTOR_BASE_ENABLE, 0);

    gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO_DIRECAO, 0);
    gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO_ENABLE, 0);

    gpio_set_level(CONFIG_GPIO_MOTOR_ESQUERDO_DIRECAO, 0);
    gpio_set_level(CONFIG_GPIO_MOTOR_ESQUERDO_ENABLE, 0);

    while (1)
    {

        vTaskDelay(1);

        if (xQueueReceive(thetas, &itens, 100))
        {
            ESP_LOGI("angles", "%lf...", itens.theta_1);
            ESP_LOGI("angles", "%lf...", itens.theta_2);
            ESP_LOGI("angles", "%lf...", itens.theta_3);
        }

        if (xQueueReceive(gpio_end_motor_base, &gpio_sensor_base, 500))
        {
            ESP_LOGI("ISR", "Fim de curso motor 1...");
            gpio_set_level(CONFIG_GPIO_MOTOR_BASE_DIRECAO, 1);
        }

        if (xQueueReceiveFromISR(gpio_end_motor_esquerdo, &gpio_sensor_left, 9))
        {
            ESP_LOGI("ISR", "Fim de curso motor 2...");
            gpio_set_level(CONFIG_GPIO_MOTOR_ESQUERDO_DIRECAO, 1);
        }

        if (xQueueReceiveFromISR(gpio_end_motor_direito, &gpio_sensor_right, 10))
        {
            ESP_LOGI("ISR", "Fim de curso motor 3...");
            gpio_set_level(CONFIG_GPIO_MOTOR_DIREITO_DIRECAO, 1);
        }
    }
}

#endif
