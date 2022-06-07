#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include <sys/param.h>

#include "uart.h"
#include "robot.h"

void app_main(void)
{

    printf("Starting...");

    init_uart();
    init_robot();

    xTaskCreate(task_robot, "controller_task_robot", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(rx_task, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 2, NULL);
    
    // uint32_t gpio_sensor_esquerdo;
    // uint32_t gpio_sensor_direito;

    // gpio_config_t end_base_esquerdo = {.pull_up_en   = 1, .mode = GPIO_MODE_INPUT, .pin_bit_mask = (1ULL<<CONFIG_GPIO_END_MOTOR_ESQUERDO) };
    // gpio_config_t end_base_direito  = { .pull_up_en   = 1, .mode = GPIO_MODE_INPUT, .pin_bit_mask = (1ULL<<CONFIG_GPIO_END_MOTOR_DIREITO) };

    // gpio_config(&end_base_esquerdo);
    // gpio_config(&end_base_direito);

    // while (1)
    // {
    //     gpio_sensor_esquerdo = gpio_get_level(CONFIG_GPIO_END_MOTOR_ESQUERDO);
    //     ESP_LOGI("ESQUERDO"," gpio_sensor_esquerdo %i", gpio_sensor_esquerdo);
        
    //     gpio_sensor_direito = gpio_get_level(CONFIG_GPIO_END_MOTOR_DIREITO);
    //     ESP_LOGI("DIREITO"," gpio_sensor_direito %i", gpio_sensor_direito);
    //     vTaskDelay(2000 / portTICK_PERIOD_MS);
    // }
    



}
