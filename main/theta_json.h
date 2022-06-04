#ifndef _THETA_JSON_
#define _THETA_JSON_

#include "cJSON.h"
#include "esp_log.h"
#include "robot.h"
#include "final_actuator.h"



void take_json(char *json_values) {
   
    cJSON *root = cJSON_Parse(json_values);
    
    if (root != NULL) {
        printf("_JSON_");
        double theta1 = cJSON_GetObjectItem(root, "theta1")->valuedouble;
        double theta2 = cJSON_GetObjectItem(root, "theta2")->valuedouble;
        double theta3 = cJSON_GetObjectItem(root, "theta3")->valuedouble;
        
        List_thetas thetas_values;
        thetas_values.theta_1 = theta1; 
        thetas_values.theta_2 = theta2; 
        thetas_values.theta_3 = theta3; 

        xQueueSend(thetas, (void *)&thetas_values, 1000); 

    }
    cJSON_Delete(root);
}

#endif