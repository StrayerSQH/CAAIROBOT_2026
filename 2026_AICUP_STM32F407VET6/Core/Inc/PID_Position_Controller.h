//
// Created by s on 26-1-29.
//

#ifndef INC_2026_AICUP_STM32F407VET6_PID_POSITION_CONTROLLER_H
#define INC_2026_AICUP_STM32F407VET6_PID_POSITION_CONTROLLER_H

#include <stdint-gcc.h>

typedef struct {
    float Kp;               // 比例增益
    float Ki;               // 积分增益
    float Kd;               // 微分增益
    float error;            // 当前误差
    float last_error;       // 上次误差
    float prev_error;       // 上上次误差（用于微分）
    float differ_error;
    float sum_error;
    float output;           // 控制器输出
    float integral_limit;   // 积分限幅
    float output_limit;     // 输出限幅
} PID_Position_Controller;

void PID_Position_Controller_Init(PID_Position_Controller *pid,
                               float Kp, float Ki, float Kd,
                               float integral_limit,
                               float output_limit);

inline float Limit_Position(float cal_value, float limit_value);

float PID_Position_Controller_Update(PID_Position_Controller *pid,
                                  float target_val,
                                  float current_val);

#endif //INC_2026_AICUP_STM32F407VET6_PID_POSITION_CONTROLLER_H
