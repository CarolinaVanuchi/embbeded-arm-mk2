#ifndef _THETA_JSON_
#define _THETA_JSON_

#include "cJSON.h"
#include "esp_log.h"
#include "esp_log.h"

#include "motor_base.h"
#include "motor_left.h"
#include "motor_right.h"

static const char *TAG_JSON = "JSON";

void take_json(char *json_values)
{

    cJSON *root = cJSON_Parse(json_values);

    if (root != NULL)
    {
        ESP_LOGI("JSON", "take_json");

        int try_theta1 = cJSON_HasObjectItem(root, "theta1");
        int try_theta2 = cJSON_HasObjectItem(root, "theta2");
        int try_theta3 = cJSON_HasObjectItem(root, "theta3");

        if (try_theta1 == 1 && try_theta2 == 1 && try_theta3 == 1)
        {
            double theta1 = cJSON_GetObjectItem(root, "theta1")->valuedouble;
            double theta2 = cJSON_GetObjectItem(root, "theta2")->valuedouble;
            double theta3 = cJSON_GetObjectItem(root, "theta3")->valuedouble;

            if ((theta1 >= 0.0) && (theta1 <= 320.0))
                xQueueSend(theta_base, (void *)&theta1, 1000);
            else
                ESP_LOGI(TAG_JSON, "intervalo de theta para base nao aceito");

            xQueueSend(theta_left, (void *)&theta2, 1000);
            xQueueSend(theta_right, (void *)&theta3, 1000);
        }
        else
        {
            ESP_LOGI(TAG_JSON, "Formato incorreto...");
        }
    }
    cJSON_Delete(root);
}

#endif