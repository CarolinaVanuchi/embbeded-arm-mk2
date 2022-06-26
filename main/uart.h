#ifndef _UART_
#define _UART_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "theta_json.h"
#include "motor_base.h"
#include "motor_left.h"
#include "motor_right.h"

static const int RX_BUF_SIZE = 1024;

void init_uart(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_driver_install(UART_NUM_2, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, CONFIG_GPIO_TXD, CONFIG_GPIO_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    gpio_set_pull_mode(CONFIG_GPIO_RXD, GPIO_FLOATING);
}

int sendData(const char *logName, const char *data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_2, data, len);
    return txBytes;
}

static void tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    while (1)
    {
        ESP_LOGI(TX_TASK_TAG, "tx_task");

        // cJSON *root = cJSON_CreateObject();

        // cJSON_AddNumberToObject(root, "theta1", theta_1_send);
        // cJSON_AddNumberToObject(root, "theta2", theta_2_send);
        // cJSON_AddNumberToObject(root, "theta3", theta_3_send);
        // char *buffer = cJSON_Print(root);
        // sendData(TX_TASK_TAG, buffer);
        // cJSON_Delete(root);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static void rx_task(void *arg)
{

    uint8_t *data = (uint8_t *)malloc(RX_BUF_SIZE + 1);

    while (1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_2, data, RX_BUF_SIZE, pdMS_TO_TICKS(500));

        if (rxBytes > 0)
        {
            ESP_LOGI("RX", "rx_task");
            char *json = (char *)calloc(rxBytes + 1, sizeof(char));
            memcpy(json, data, rxBytes);
            json[rxBytes] = '\0';
            take_json(json);
            free(json);
        }
        
        esp_task_wdt_reset();  
    }
    free(data);
}

#endif