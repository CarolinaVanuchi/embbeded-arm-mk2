#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include <sys/param.h>
#include "final_actuator.h"
#include "led.h"
#include "uart.h"

void app_main(void)
{
         
    printf("Starting...");
    
    set_config_final_actuator();
    set_config_led();
    init_uart();
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}

