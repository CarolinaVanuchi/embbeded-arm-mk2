#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "theta_json.h"

static const int RX_BUF_SIZE = 1024;

void init_uart(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, CONFIG_GPIO_TXD, CONFIG_GPIO_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    gpio_set_pull_mode(CONFIG_GPIO_RXD, GPIO_FLOATING);
}

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    return txBytes;
}

static void tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    while (1) {
        sendData(TX_TASK_TAG, "Enviando");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

static void rx_task(void *arg)
{
 
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        
        if (rxBytes > 0) {
            char *json = (char*)calloc(rxBytes+1, sizeof(char));
            memcpy(json, data, rxBytes);
            json[rxBytes] = '\0';
            
            take_json(json);

            free(json);
        }
    }
    free(data);
}
