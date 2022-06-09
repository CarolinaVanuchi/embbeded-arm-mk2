#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include <sys/param.h>

#include "uart.h"
#include "robot.h"
#include "end_base.h"

void app_main(void)
{

    printf("Starting...");

    init_robot();
    init_end_base();
    init_uart();

    xTaskCreate(rx_task, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES -3, NULL);
    xTaskCreate(task_robot, "controller_task_robot", 1024 * 2, NULL, configMAX_PRIORITIES -1, NULL);
    xTaskCreate(task_end_base, "controller_task_end_base", 1024 * 2, NULL, configMAX_PRIORITIES -2, NULL);
}
