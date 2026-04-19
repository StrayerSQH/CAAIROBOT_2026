//
// Created by s on 26-1-17.
//

#include <math.h>
#include "MecanumWheelPlatform.h"

float H = 0.32;
float W = 0.4;

PID_Position_Controller angle_circle;
PID_Speed_Controller angular_circle;

Chassis_Speed_Controller chassis_ctrl;

Last_Angular last_angular = {0};
bool ifBeginAngularCal = false;

M3508 motor[4];

uint16_t motor_A_ID = 0x201;
uint16_t motor_B_ID = 0x202;
uint16_t motor_C_ID = 0x203;
uint16_t motor_D_ID = 0x204;

/**
  * @brief 设置CAN滤波器并启动CAN通信.
  */
void CAN_Filter_Init(void) {
    CAN_FilterTypeDef can_filter_st;
    can_filter_st.FilterActivation = ENABLE;
    can_filter_st.FilterMode = CAN_FILTERMODE_IDMASK;
    can_filter_st.FilterScale = CAN_FILTERSCALE_32BIT;
    can_filter_st.FilterIdHigh = 0x0000;
    can_filter_st.FilterIdLow = 0x0000;
    can_filter_st.FilterMaskIdHigh = 0x0000;
    can_filter_st.FilterMaskIdLow = 0x0000;
    can_filter_st.FilterBank = 0;
    can_filter_st.FilterFIFOAssignment = CAN_RX_FIFO0;
    HAL_CAN_ConfigFilter(&hcan1, &can_filter_st);
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

/**
  * @brief 初始化麦克纳姆轮底盘（包括陀螺仪、电机等模块）.
  */
void MecnaumuWheelPlatform_Init(void) {
    IMU406_Init(); //初始化IMU406陀螺仪模块

    PID_Speed_Controller_Init(&chassis_ctrl.linear_x_pid,
                              5.0f, 0.1f, 2.0f,
                              2.0f, 2.0f);
    PID_Speed_Controller_Init(&chassis_ctrl.linear_y_pid,
                              5.0f, 0.1f, 2.0f,
                              2.0f, 2.0f);
    PID_Speed_Controller_Init(&chassis_ctrl.angular_z_pid,
                              1.0f, 0.1f, 0.1f,
                              3.14f, 3.14f);

    last_angular.last_w_x = (float) IMU406_YAW / 100;
    ifBeginAngularCal = true;

//    PID_Position_Controller_Init(&angle_circle, 1, 1, 1, 360, 60);
//    PID_Speed_Controller_Init(&angular_circle, 1, 1, 1, 60, 60);

    //初始化M3508电机
    M3508_Init(&motor[0], motor_A_ID);
    M3508_Init(&motor[1], motor_B_ID);
    M3508_Init(&motor[2], motor_C_ID);
    M3508_Init(&motor[3], motor_D_ID);

    CAN_Filter_Init();
    ROS_Init();
}

/**
 * @brief ROS期望麦克纳姆轮底盘的运动结果会和实际运动结果有出入，因此需要底盘级PID进行闭环控制
 */
static void Chassis_PID_Update(void) {
    chassis_ctrl.linear_x_output = PID_Speed_Controller_Update(&chassis_ctrl.linear_x_pid,
                                                          f_ROS.linear_x,
                                                          t_ROS.linear_x);

    chassis_ctrl.linear_y_output = PID_Speed_Controller_Update(&chassis_ctrl.linear_y_pid,
                                                          f_ROS.linear_y,
                                                          t_ROS.linear_y);

    chassis_ctrl.angular_z_output = PID_Speed_Controller_Update(&chassis_ctrl.angular_z_pid,
                                                          f_ROS.angular_z,
                                                          t_ROS.angular_z);
}

/**
 * @brief 由于IMU406输出的角速度波动较大，因此每10ms通过人工计算的方式求解角速度
 */
void Cal_Angular_IT(void) {
    float t = (float) IMU406_YAW / 100;
    t_ROS.angular_x = (t - last_angular.last_w_x) / 0.01;
    last_angular.last_w_x = t;
}

/**
 * @brief 每10ms计算一次麦克纳姆轮底盘正运动学更新t_ROS信息
 */
void Forward_Kinematics_IT(void) {
    t_ROS.linear_x = (motor[0].current_speed
                 - motor[1].current_speed
                 - motor[2].current_speed
                 + motor[3].current_speed) / 4;
    t_ROS.linear_y = (-motor[0].current_speed
                 - motor[1].current_speed
                 + motor[2].current_speed
                 + motor[3].current_speed) / 4;

    t_ROS.linear_z = 0;

    t_ROS.angular_x = 0;
    t_ROS.angular_y = 0;
    t_ROS.angular_z = (-motor[0].current_speed
                 - motor[1].current_speed
                 - motor[2].current_speed
                 - motor[3].current_speed) / (H + W);
}

/**
 * @brief 每10ms根据f_ROS信息计算麦克纳姆轮底盘控制信息
 */
void Inverse_Kinematics_IT(void) {
    Chassis_PID_Update();

    motor[0].ideal_speed = chassis_ctrl.linear_x_output
                           - chassis_ctrl.linear_y_output
                           - chassis_ctrl.angular_z_output * (H + W) / 2.0f;

    motor[1].ideal_speed = -chassis_ctrl.linear_x_output
                           - chassis_ctrl.linear_y_output
                           - chassis_ctrl.angular_z_output * (H + W) / 2.0f;

    motor[2].ideal_speed = -chassis_ctrl.linear_x_output
                           + chassis_ctrl.linear_y_output
                           - chassis_ctrl.angular_z_output * (H + W) / 2.0f;

    motor[3].ideal_speed = chassis_ctrl.linear_x_output
                           + chassis_ctrl.linear_y_output
                           - chassis_ctrl.angular_z_output * (H + W) / 2.0f;

    Wheels_PID_Update();
}

/**
 * @brief 被void Inverse_Kinematics_IT(void)函数调用对每一个轮子进行PID闭环控制
 */
static void Wheels_PID_Update(void) {
    motor[0].ideal_current = (int16_t) PID_Speed_Controller_Update(&motor[0].speed_circle,
                                                                   motor[0].ideal_speed,
                                                                   motor[0].current_speed);
    motor[1].ideal_current = (int16_t) PID_Speed_Controller_Update(&motor[1].speed_circle,
                                                                   motor[1].ideal_speed,
                                                                   motor[1].current_speed);
    motor[2].ideal_current = (int16_t) PID_Speed_Controller_Update(&motor[2].speed_circle,
                                                                   motor[2].ideal_speed,
                                                                   motor[2].current_speed);
    motor[3].ideal_current = (int16_t) PID_Speed_Controller_Update(&motor[3].speed_circle,
                                                                   motor[3].ideal_speed,
                                                                   motor[3].current_speed);
    CAN_CMD_Chassis();
}

/**
 * @brief 被static void Wheels_PID_Update(void)函数调用将计算出的电流信息通过CAN信号发送至M3508电机
 */
static void CAN_CMD_Chassis(void)
{
    uint32_t send_mailbox;
    chassis_tx_message.StdId = CHASSIS_CONTROL_ID;
    chassis_tx_message.IDE = CAN_ID_STD;
    chassis_tx_message.RTR = CAN_RTR_DATA;
    chassis_tx_message.DLC = 0x08;

    chassis_can_send_data[0] = motor[0].ideal_current >> 8;
    chassis_can_send_data[1] = motor[0].ideal_current;
    chassis_can_send_data[2] = motor[1].ideal_current >> 8;
    chassis_can_send_data[3] = motor[1].ideal_current;
    chassis_can_send_data[4] = motor[2].ideal_current >> 8;
    chassis_can_send_data[5] = motor[2].ideal_current;
    chassis_can_send_data[6] = motor[3].ideal_current >> 8;
    chassis_can_send_data[7] = motor[3].ideal_current;

    HAL_CAN_AddTxMessage(&hcan1,
                         &chassis_tx_message,
                         chassis_can_send_data,
                         &send_mailbox);
}
/**
 * @brief 解析M3508电机返回的CAN数据并更新电机状态
 * @param m3508 指向M3508电机结构体的指针，用于存储解析后的数据
 * @param rx_data CAN接收到的8字节原始数据数组
 */
static void Get_Motor_Data(M3508 *m3508, uint8_t rx_data[]) {
    m3508->current_position = ((rx_data[0] << 8) | rx_data[1]) * 360.0f / 8191.0f;;
    m3508->current_speed = Rpm_To_Linear_Velocity((rx_data[2] << 8) | rx_data[3]);
    m3508->measured_current = (rx_data[4] << 8) | rx_data[5];
    m3508->current_temp = rx_data[6];
}

/**
 * @brief CAN接收FIFO0消息挂起回调函数
 * @param hcan CAN句柄指针，指示哪个CAN外设触发了中断
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);

    uint8_t motor_ID = 0;
    motor_ID = rx_header.StdId - motor_A_ID;        // 获取电调ID
    if (motor_ID >= 0 && motor_ID <= 3) {
        Get_Motor_Data(&motor[motor_ID], rx_data);
    }
}