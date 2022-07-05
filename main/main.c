#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include <sys/param.h>
#include "soc/rtc_wdt.h"

#include "uart.h"
#include "sensor.h"

#include "motor_base.h"
#include "motor_left.h"
#include "motor_right.h"
#include "led.h"
#include "final_actuator.h"

void app_main(void)
{

    ESP_LOGI("MK2 Plus", "Start...");

    init_uart();
    init_sensor();

    set_config_led();

    init_final_actuator();
   
    xTaskCreate(task_motor_base, "task_base", 1024 * 20, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(task_motor_left, "task_left", 1024 * 20, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(task_motor_right, "task_right", 1024 * 20, NULL, configMAX_PRIORITIES - 3, NULL);

    xTaskCreate(task_sensor, "task_sensor", 1024 * 5, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(rx_task, "uart_rx_task", 1024 * 5, NULL, configMAX_PRIORITIES - 2, NULL);
    xTaskCreate(close_final_actuator, "task_garra", 1024 * 5, NULL, configMAX_PRIORITIES - 4, NULL);
}
