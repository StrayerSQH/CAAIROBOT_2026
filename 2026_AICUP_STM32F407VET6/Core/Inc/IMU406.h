//
// Created by s on 26-1-17.
//
#include <stdint.h>
#include <stdbool.h>
#include "usart.h"

#ifndef INC_2026_AICUP_STM32F407VET6_IMU406_H
#define INC_2026_AICUP_STM32F407VET6_IMU406_H

typedef int32_t IMU406; //类型别名定义

/* ------------------------------------------------------------------ */
/*                     偏航角、翻滚角、俯仰角等定义                        */
/* ------------------------------------------------------------------ */
extern volatile IMU406 IMU406_ROLL;      // 翻滚角
extern volatile IMU406 IMU406_PITCH;     // 俯仰角
extern volatile IMU406 IMU406_YAW;       // 偏航角
extern volatile IMU406 IMU406_ACC_X;     // X轴加速度
extern volatile IMU406 IMU406_ACC_Y;     // Y轴加速度
extern volatile IMU406 IMU406_ACC_Z;     // Z轴加速度
extern volatile IMU406 IMU406_GYRO_X;    // X轴角速度
extern volatile IMU406 IMU406_GYRO_Y;    // Y轴角速度
extern volatile IMU406 IMU406_GYRO_Z;    // Z轴角速度

/* ------------------------------------------------------------------ */
/*                     配置区：根据 IMU406协议                           */
/* ------------------------------------------------------------------ */
#define IMU406_FRAME_LEN     29U
#define IMU406_FRAME_HEAD    0x84

/* ------------------------------------------------------------------ */
/*                   接收状态机私有变量                                  */
/* ------------------------------------------------------------------ */
static uint8_t  g_rxByte        = 0; //单字节缓冲
static uint8_t  s_rxBuf[IMU406_FRAME_LEN]; //帧缓冲数组
static uint8_t  s_rxIndex       = 0; //当前帧缓冲数组下标

void IMU406_Init(void); //初始化IMU406
void IMU406_Rx_ISR(void); //接受串口终端
static void IMU406_ParseFrame(const uint8_t *frame); //解析完整IMU406_FRAME_LEN-Byte流

static inline IMU406 DecodeBytes(const uint8_t *p); //解析3位字符代表的数据

/* 单数据获取函数 */
IMU406 IMU406_Get_Roll(void);
IMU406 IMU406_Get_Pitch(void);
IMU406 IMU406_Get_Yaw(void);
IMU406 IMU406_Get_AccX(void);
IMU406 IMU406_Get_AccY(void);
IMU406 IMU406_Get_AccZ(void);
IMU406 IMU406_Get_GyroX(void);
IMU406 IMU406_Get_GyroY(void);
IMU406 IMU406_Get_GyroZ(void);

#endif //INC_2026_AICUP_STM32F407VET6_IMU406_H
