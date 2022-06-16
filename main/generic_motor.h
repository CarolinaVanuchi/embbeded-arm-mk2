#ifndef _GENERIC_MOTOR_
#define _GENERIC_MOTOR_

#define FREQUENCY_MAX (250)

double get_new_theta(double get_new_theta, double old_theta, uint8_t horario, uint8_t anti_horario, gpio_num_t dir_port)
{
    double aux_left = get_new_theta - old_theta;
    if (aux_left > 0)
    {
        gpio_set_level(dir_port, horario);
        return aux_left;
    }
    else if (aux_left < 0)
    {
        gpio_set_level(dir_port, anti_horario);
        return (aux_left * (-1));
    }

    return 0;
}


double get_end_time(double angle, int hertz, int step, double gear)
{
    double passos = ((200 * 8 * angle) / 360);
    return (passos / hertz) * 1000000;
}

#endif