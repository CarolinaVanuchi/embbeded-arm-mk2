#ifndef _LED_
#define _LED_


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"


static void blink_led(uint8_t status)
{
   
    gpio_set_level(CONFIG_GPIO_LED, status);
}

static void set_config_led(void)
{
    gpio_reset_pin(CONFIG_GPIO_LED);
    gpio_set_direction(CONFIG_GPIO_LED, GPIO_MODE_OUTPUT);
}

void on_led() {
  blink_led(1);  
}

void off_led() {
    blink_led(0);
}

#endif