//
// Created by s on 26-4-12.
//
#include "servo.h"


void Servo_Init(Servo *servo, TIM_HandleTypeDef *htim, uint32_t channel){
    servo->htim = htim;
    servo->Channel = channel;
    HAL_TIM_PWM_Start(htim, channel);
    __HAL_TIM_SET_COMPARE(htim, channel, 0);
}

void Servo_Set_Angel(Servo *servo, int angel) {
    int compare = 50 + angel * 200 / 180;
    __HAL_TIM_SET_COMPARE(servo->htim, servo->Channel, compare);
}
