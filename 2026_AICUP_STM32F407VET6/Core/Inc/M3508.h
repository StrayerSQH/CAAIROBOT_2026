//
// Created by s on 26-1-19.
//

#ifndef INC_2026_AICUP_STM32F407VET6_M3508_H
#define INC_2026_AICUP_STM32F407VET6_M3508_H

#include <stdint-gcc.h>
#include "stm32f4xx_hal.h"
#include "PID_Speed_Controller.h"

extern float WHEEL_D;

typedef struct {
    uint16_t motor_can_id;

    float ideal_speed;
    int16_t ideal_current;

    float current_position;
    float current_speed;
    int16_t measured_current;
    float current_temp;

    PID_Speed_Controller speed_circle;
} M3508;

void M3508_Init(M3508 *m3508, uint16_t motor_can_id);

float Rpm_To_Linear_Velocity(int16_t rpm);

#endif //INC_2026_AICUP_STM32F407VET6_M3508_H
