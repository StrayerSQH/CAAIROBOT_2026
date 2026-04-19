//
// Created by s on 26-2-22.
//
#include "ROS.h"

From_ROS_Platform_Message f_ROS = {0};
To_ROS_Platform_Message t_ROS = {0};

/**
 * @brief 初始化ROS串口通信
 */
void ROS_Init(void) {
    /* 初始化串口硬件 */
    MX_USART1_UART_Init();

    /* 开启串口接收中断 */
    HAL_UART_Receive_IT(&huart1, &ros_rxByte, 1);

    f_ROS.linear_x = 0;
    f_ROS.linear_y = 0;
    f_ROS.angular_z = 0;
}

/**
 * @brief ROS数据接收中断服务函数
 * 在串口中断中调用
 */
void ROS_Receive_From_ROS_ISR() {
    if (ros_rxIndex == 0U) {
        if (ros_rxByte != ROS_RECEIVE_FRAME_HEAD) {
            goto _next;
        }
    }

    ros_rxBuf[ros_rxIndex++] = ros_rxByte;

    if (ros_rxIndex >= ROS_RECEIVE_FRAME_LEN) {
        ROS_Receive_ParseFrame(ros_rxBuf);
        ros_rxIndex = 0U;
    }

    _next:
    HAL_UART_Receive_IT(&huart1, &ros_rxByte, 1);
}

/**
 * @brief 解析从ROS接收到的数据帧
 * @param frame 接收到的数据帧缓冲区指针
 */
static void ROS_Receive_ParseFrame(const uint8_t *frame) {
    const float *pData = (const float*)&frame[1];
    f_ROS.linear_x = pData[0];
    f_ROS.linear_y = pData[1];
    f_ROS.linear_z = 0;         // 强制赋0，防止报错
    f_ROS.angular_x = 0;        // 强制赋0，防止报错
    f_ROS.angular_y = 0;        // 强制赋0，防止报错
    f_ROS.angular_z = pData[5];
}

/**
 * @brief 发送数据到ROS平台
 * 将t_ROS结构体中的数据通过串口发送给ROS
 */
void ROS_Transit_To_ROS(void) {
    uint8_t t_ROS_message[ROS_TRANSIT_FRAME_LEN];
    t_ROS_message[0] = ROS_TRANSIT_FRAME_HEAD;

    float *pData = (float*)&t_ROS_message[1];
    pData[0] = t_ROS.linear_x;
    pData[1] = t_ROS.linear_y;
    pData[2] = t_ROS.linear_z;
    pData[3] = t_ROS.angular_x;
    pData[4] = t_ROS.angular_y;
    pData[5] = t_ROS.angular_z;

    HAL_UART_Transmit(&huart1, t_ROS_message, ROS_TRANSIT_FRAME_LEN, 10);
}
