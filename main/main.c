#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include <sys/param.h>
#include "final_actuator.h"


void app_main(void)
{
    printf("Starting...");
    set_config_final_actuator();
    open_final_actuator();
    close_final_actuator();
  
}