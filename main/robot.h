#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define BLINK_GPIO (2)

static void blink_led(uint8_t status)
{
   
    gpio_set_level(BLINK_GPIO, status);
}

static void set_config_led(void)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

void on_led() {
  blink_led(1);  
}

void off_led() {
    blink_led(0);
}
