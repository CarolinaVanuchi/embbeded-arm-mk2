#ifndef _GENERIC_MOTOR_
#define _GENERIC_MOTOR_

double get_new_theta(double get_new_theta, double old_theta, uint8_t horario, uint8_t anti_horario, gpio_num_t dir_port)
{
    double aux_left = get_new_theta - old_theta;
    
    ESP_LOGI("EX", "%f", aux_left);
    ESP_LOGI("EX1", "%f", get_new_theta);
    ESP_LOGI("EX2", "%f", old_theta);

    if (aux_left > 0)
    {
        gpio_set_level(dir_port, horario);
        return aux_left;
    }
    else if (aux_left < 0)
    {
        gpio_set_level(dir_port, anti_horario);
        return (aux_left * (-1));
    } else if (get_new_theta == 0) {
         gpio_set_level(dir_port, anti_horario);
         return old_theta;
    }

    return 0;
}

double get_step(double angle, int step_value, double gear, uint8_t diff_motor)
{
    return ((200 * step_value * angle * gear * diff_motor) / 360);
}


#endif