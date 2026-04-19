//
// Created by s on 26-1-19.
//
#include "M3508.h"

float WHEEL_D = 0.05;

void M3508_Init(M3508 *m3508, uint16_t motor_can_id) {
    m3508->motor_can_id = motor_can_id;

    m3508->ideal_speed = 0;
    m3508->ideal_current = 0;

    m3508->current_position = 0;
    m3508->current_speed = 0;
    m3508->measured_current = 0;
    m3508->current_temp = 0;

    PID_Speed_Controller_Init(&m3508->speed_circle, 2000, 30, 1000, 16384, 16384);
}

float Rpm_To_Linear_Velocity(int16_t rpm) {
    return 2 * 3.1415926 * WHEEL_D * rpm / (19 * 60);
}

