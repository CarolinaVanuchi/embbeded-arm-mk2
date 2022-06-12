#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include <sys/param.h>

#include "uart.h"
#include "robot.h"
#include "end_base.h"
#include "motor_base.h"
#include "motor_left.h"
#include "motor_right.h"

#define BLINK_GPIO (22)
void app_main(void)
{
    printf("Starting...");

    init_robot();
    init_end_base();
    init_uart();

    xTaskCreate(task_robot, "controller_task_robot", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(task_motor_base, "controller_task_motor_base", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(task_motor_left, "controller_task_motor_left", 1024 * 2, NULL, configMAX_PRIORITIES - 2, NULL);
    xTaskCreate(task_motor_right, "controller_task_motor_right", 1024 * 2, NULL, configMAX_PRIORITIES - 3, NULL);

    xTaskCreate(task_end_base, "controller_task_end_base", 1024 * 2, NULL, configMAX_PRIORITIES - 4, NULL);
    xTaskCreate(rx_task, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 5, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 6, NULL);
}
