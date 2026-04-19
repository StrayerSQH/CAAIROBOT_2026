//
// Created by s on 26-4-12.
//

#ifndef ARM_CONTROL_BOARD_SERVO_H
#define ARM_CONTROL_BOARD_SERVO_H

#include "tim.h"

typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t Channel;
} Servo;

void Servo_Init(Servo *servo, TIM_HandleTypeDef *htim, uint32_t channel);

void Servo_Set_Angel(Servo *servo, int angel);

#endif //ARM_CONTROL_BOARD_SERVO_H
