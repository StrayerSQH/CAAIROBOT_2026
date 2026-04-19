//
// Created by s on 26-1-17.
//

#ifndef INC_2026_AICUP_STM32F407VET6_PID_SPEED_CONTROLLER_H
#define INC_2026_AICUP_STM32F407VET6_PID_SPEED_CONTROLLER_H

#include <stdint-gcc.h>

typedef struct {
    float Kp;               // 比例增益
    float Ki;               // 积分增益
    float Kd;               // 微分增益
    float integral;         // 积分项累积
    float error;            // 当前误差
    float last_error;       // 上次误差
    float prev_error;       // 上上次误差（用于微分）
    float output;           // 控制器输出
    float integral_limit;   // 积分限幅
    float output_limit;     // 输出限幅
} PID_Speed_Controller;

void PID_Speed_Controller_Init(PID_Speed_Controller *pid,
                                  float Kp, float Ki, float Kd,
                                  float integral_limit,
                                  float output_limit);

float PID_Speed_Controller_Update(PID_Speed_Controller *pid,
                                  float target_val,
                                  float current_val);

#endif //INC_2026_AICUP_STM32F407VET6_PID_SPEED_CONTROLLER_H
