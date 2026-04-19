//
// Created by s on 26-2-22.
//
#include "tim.h"
#include "usart.h"

#ifndef INC_2026_AICUP_STM32F407VET6_ROS_H
#define INC_2026_AICUP_STM32F407VET6_ROS_H

#define ROS_RECEIVE_FRAME_LEN     25U
#define ROS_RECEIVE_FRAME_HEAD    0x52     // 从ROS端接受帧理论帧头

#define ROS_TRANSIT_FRAME_LEN     25U
#define ROS_TRANSIT_FRAME_HEAD    0x54      // 向ROS端发送帧帧头

static uint8_t  ros_rxByte = 0;
static uint8_t  ros_rxBuf[ROS_RECEIVE_FRAME_LEN];
static uint8_t ros_rxIndex = 0;

typedef struct {
    float linear_x;
    float linear_y;
    float linear_z;

    float angular_x;
    float angular_y;
    float angular_z;
} From_ROS_Platform_Message;    //从ROS获取的geometry_msgs/Twist控制消息

typedef struct {
    float linear_x;
    float linear_y;
    float linear_z;

    float angular_x;
    float angular_y;
    float angular_z;
} To_ROS_Platform_Message;      //发送给ROS的geometry_msgs/Twist车辆信息

extern From_ROS_Platform_Message f_ROS; //ROS发来的控制信息
extern To_ROS_Platform_Message t_ROS; //发送给ROS的底盘信息

void ROS_Init(void);

void ROS_Receive_From_ROS_ISR(void);
static void ROS_Receive_ParseFrame(const uint8_t *frame);

void ROS_Transit_To_ROS(void);

#endif //INC_2026_AICUP_STM32F407VET6_ROS_H
