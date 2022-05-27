#include "cJSON.h"
#include "esp_log.h"
#include "robot.h"
void take_json(char *json_values) {
    
    cJSON *root = cJSON_Parse(json_values);
    
    int theta1 = cJSON_GetObjectItem(root, "theta1")->valueint;
    int theta2 = cJSON_GetObjectItem(root, "theta2")->valueint;
    int theta3 = cJSON_GetObjectItem(root, "theta3")->valueint;
    
    if(theta1==22 && theta2==60 && theta3==90){
        set_config_led();
        on_led();
    }
   
    cJSON_Delete(root);
}