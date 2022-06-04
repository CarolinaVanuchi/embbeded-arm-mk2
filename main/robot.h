#ifndef _ROBOT_
#define _ROBOT_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define ESP_INTR_FLAG_DEFAULT 0
typedef struct {
    double theta_1;
    double theta_2;
    double theta_3;
} List_thetas;

static xQueueHandle gpio_end_motor_esquerdo  = NULL;
static xQueueHandle thetas              = NULL;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_sensor = (uint32_t) arg;
    xQueueSendFromISR(gpio_end_motor_esquerdo, &gpio_sensor, NULL);
}


void init_motor_esquerdo(void)
{
  
    gpio_reset_pin(CONFIG_GPIO_END_MOTOR_ESQUERDO);
    gpio_set_direction(CONFIG_GPIO_END_MOTOR_ESQUERDO, GPIO_MODE_INPUT);

    gpio_config_t config_end_base = {
        .intr_type      = GPIO_PIN_INTR_POSEDGE,
        .pin_bit_mask   = (1ULL<<CONFIG_GPIO_END_MOTOR_ESQUERDO),
        .mode           = GPIO_MODE_INPUT,
        .pull_down_en   = 1,
    };

    gpio_config(&config_end_base);

    gpio_end_motor_esquerdo = xQueueCreate(1, sizeof(uint32_t));
    gpio_isr_handler_add(CONFIG_GPIO_END_MOTOR_ESQUERDO, gpio_isr_handler, (void *)CONFIG_GPIO_END_MOTOR_ESQUERDO);

    thetas = xQueueCreate(3, sizeof(List_thetas));
}

static void init_robot(void)
{
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    init_motor_esquerdo();
}

static void task_robot(void *arg) {
   
    uint32_t gpio_sensor;
    List_thetas itens;
    while (1)
    {
       
        if (xQueueReceive(thetas, &itens, 100)) {
            ESP_LOGI("angles", "%lf...", itens.theta_1);
            ESP_LOGI("angles", "%lf...", itens.theta_2);
            ESP_LOGI("angles", "%lf...", itens.theta_3);
        }
       
        if (xQueueReceive(gpio_end_motor_esquerdo, &gpio_sensor, 100)) {
           ESP_LOGI("isr", "Fim de curso motor 2...");
        }
     
    }
    
}

#endif
