#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include <sys/param.h>
#include "soc/rtc_wdt.h"

#include "uart.h"
#include "sensor.h"
#include "end_base.h"

#include "motor_base.h"
#include "motor_left.h"
#include "motor_right.h"
#include "led.h"

void app_main(void)
{
    ESP_LOGI("MK2 Plus", "Start...");
    rtc_wdt_protect_off();
    rtc_wdt_disable();

    init_uart();
    init_sensor();
    init_end_base();

    set_config_led();
    on_led();

    
    xTaskCreate(task_sensor, "task_sensor", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);

    xTaskCreate(task_motor_base, "task_base", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(task_motor_left, "task_left", 1024 * 2, NULL, configMAX_PRIORITIES - 2, NULL);
    xTaskCreate(task_motor_right, "task_right", 1024 * 2, NULL, configMAX_PRIORITIES - 3, NULL);

    xTaskCreate(tx_task, "uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 6, NULL);
    xTaskCreate(task_end_base, "task_end_base", 1024 * 2, NULL, configMAX_PRIORITIES - 4, NULL);
    xTaskCreate(rx_task, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 5, NULL);
}
