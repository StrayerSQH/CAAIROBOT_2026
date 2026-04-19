//
// Created by s on 26-1-17.
//
#include "PID_Speed_Controller.h"

/**
 * @brief 初始化PID速度控制器（带限幅参数）
 * @param[out] pid PID控制器结构体指针
 * @param[in] Kp 比例系数
 * @param[in] Ki 积分系数
 * @param[in] Kd 微分系数
 * @param[in] integral_limit 积分限幅值
 * @param[in] output_limit 输出限幅值
 */
void PID_Speed_Controller_Init(PID_Speed_Controller *pid,
                                  float Kp, float Ki, float Kd,
                                  float integral_limit,
                                  float output_limit) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->integral   = 0.0;
    pid->error      = 0.0;
    pid->last_error = 0.0;
    pid->prev_error = 0.0;
    pid->output     = 0.0;
    pid->integral_limit = integral_limit;
    pid->output_limit   = output_limit;
}

/**
 * @brief PID速度环更新
 * @param[out] pid PID控制器结构体指针
 * @param[in] target_val 目标值
 * @param[in] current_val 当前值
 * @return output 输出
 */
float PID_Speed_Controller_Update(PID_Speed_Controller *pid,
                                  float target_val,
                                  float current_val) {
    pid->error = target_val - current_val;
    pid->output += pid->Kp * (pid->error - pid->last_error) +
                   pid->Ki * pid->error +
                   pid->Kd * (pid->error + pid->prev_error - 2 * pid->last_error);

    pid->prev_error = pid->last_error;
    pid->last_error = pid->error;

    if (pid->output > pid->output_limit) pid->output = pid->output_limit;
    if (pid->output < -pid->output_limit) pid->output = -pid->output_limit;

    return pid->output;
}


