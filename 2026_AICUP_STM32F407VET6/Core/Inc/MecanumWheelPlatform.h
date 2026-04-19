//
// Created by s on 26-1-17.
//

#ifndef INC_2026_AICUP_STM32F407VET6_MECANUMWHEELPLATFORM_H
#define INC_2026_AICUP_STM32F407VET6_MECANUMWHEELPLATFORM_H

#include "IMU406.h"
#include "M3508.h"
#include "can.h"
#include "Bluetooth.h"
#include "main.h"
#include "ROS.h"
#include "PID_Position_Controller.h"
#include "PID_Speed_Controller.h"

typedef struct {
    PID_Speed_Controller linear_x_pid;  // x方向速度PID
    PID_Speed_Controller linear_y_pid;  // y方向速度PID
    PID_Speed_Controller angular_z_pid;  // 绕z轴角速度PID

    float linear_x_output;      // PID输出的x方向速度补偿
    float linear_y_output;      // PID输出的y方向速度补偿
    float angular_z_output;      // PID输出的角速度补偿
} Chassis_Speed_Controller;

typedef struct {
    float last_w_x;
    float last_w_y;
    float last_w_z;
} Last_Angular;

extern float H,W;

extern Chassis_Speed_Controller chassis_ctrl;

extern Last_Angular last_angular; //村粗上一次角度的信息
extern bool ifBeginAngularCal; //是否开始计算角速度（防止启动时第一个中断计算出来的值很大）

extern M3508 motor[4];

extern uint16_t motor_A_ID, motor_B_ID, motor_C_ID, motor_D_ID;

static uint16_t CHASSIS_CONTROL_ID= 0x200;
static CAN_TxHeaderTypeDef  chassis_tx_message;
static uint8_t chassis_can_send_data[8];

void CAN_Filter_Init(void);
void MecnaumuWheelPlatform_Init(void);

static void Chassis_PID_Update(void);
void Cal_Angular_IT(void);
void Forward_Kinematics_IT(void);
void Inverse_Kinematics_IT(void);

static void Wheels_PID_Update(void);
static void CAN_CMD_Chassis(void);

static void Get_Motor_Data(M3508 *m3508, uint8_t rx_data[]);

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

#endif //INC_2026_AICUP_STM32F407VET6_MECANUMWHEELPLATFORM_H
