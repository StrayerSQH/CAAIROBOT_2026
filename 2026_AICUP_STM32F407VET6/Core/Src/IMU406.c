//
// Created by s on 26-1-17.
//
#include "IMU406.h"

/* ------------------------------------------------------------------ */
/*                    IMU406全局变量定义（实际存储区）                    */
/* ------------------------------------------------------------------ */
volatile IMU406 IMU406_ROLL   = 0;     // 翻滚角
volatile IMU406 IMU406_PITCH  = 0;     // 俯仰角
volatile IMU406 IMU406_YAW    = 0;     // 偏航角
volatile IMU406 IMU406_ACC_X  = 0;     // X轴加速度
volatile IMU406 IMU406_ACC_Y  = 0;     // Y轴加速度
volatile IMU406 IMU406_ACC_Z  = 0;     // Z轴加速度
volatile IMU406 IMU406_GYRO_X = 0;     // X轴角速度
volatile IMU406 IMU406_GYRO_Y = 0;     // Y轴角速度
volatile IMU406 IMU406_GYRO_Z = 0;     // Z轴角速度

/* ------------------------------------------------------------------ */
/*                   公共函数实现                                       */
/* ------------------------------------------------------------------ */
void IMU406_Init(void)
{
    /* 初始化串口硬件 */
    MX_USART3_UART_Init();

    /* 开启串口接收中断 */
    HAL_UART_Receive_IT(&huart3, &g_rxByte, 1);
}

void IMU406_Rx_ISR(void)
{
    /* 1. 帧头同步 */
    if (s_rxIndex == 0U) {
        if (g_rxByte != IMU406_FRAME_HEAD) {
            goto _next;                  /* 不是 0x84，丢弃 */
        }
    }

    /* 2. 写入缓冲 */
    s_rxBuf[s_rxIndex++] = g_rxByte;

    /* 3. 收满 29 字节时尝试解析 */
    if (s_rxIndex >= IMU406_FRAME_LEN) {
        IMU406_ParseFrame(s_rxBuf);
        s_rxIndex = 0U;                  /* 解析完成，重新同步 */
    }

    _next:
    /* 4. 重新开启接收中断 */
    HAL_UART_Receive_IT(&huart3, &g_rxByte, 1);
}

/* ------------------------------------------------------------------ */
/*                   私有函数实现                                       */
/* ------------------------------------------------------------------ */
static void IMU406_ParseFrame(const uint8_t *frame)
{
    const uint8_t *p = &frame[1];       /* 跳过帧头0x84 */

    /* 1. 解析姿态角：翻滚角、俯仰角、偏航角（各3字节） */
    IMU406_ROLL  = DecodeBytes(p);      p += 3;
    IMU406_PITCH = DecodeBytes(p);      p += 3;
    IMU406_YAW   = DecodeBytes(p);      p += 3;

    /* 2. 解析加速度：X、Y、Z轴（各3字节） */
    IMU406_ACC_X = DecodeBytes(p);      p += 3;
    IMU406_ACC_Y = DecodeBytes(p);      p += 3;
    IMU406_ACC_Z = DecodeBytes(p);      p += 3;

    /* 3. 解析角速度：X、Y、Z轴（各3字节） */
    IMU406_GYRO_X = DecodeBytes(p);     p += 3;
    IMU406_GYRO_Y = DecodeBytes(p);     p += 3;
    IMU406_GYRO_Z = DecodeBytes(p);
}

static inline IMU406 DecodeBytes(const uint8_t *p)
{
    /* 判断正负：最高位为1表示负数 */
    IMU406 neg = (p[0] & 0xF0) ? 1U : 0U;

    /* 获取数值的绝对值（假设范围为-18000~18000） */
    IMU406 val =
            (p[0] & 0x0F) * 10000U +
            (p[1] >> 4) * 1000U +
            (p[1] & 0x0F) * 100U +
            (p[2] >> 4) * 10U +
            (p[2] & 0x0F);

    /* 根据正负号返回实际值 */
    return neg ? -(IMU406) val : (IMU406) val;
}

/* ------------------------------------------------------------------ */
/*                   单数据获取函数实现                                 */
/* ------------------------------------------------------------------ */
IMU406 IMU406_Get_Roll(void)
{
    return IMU406_ROLL;
}

IMU406 IMU406_Get_Pitch(void)
{
    return IMU406_PITCH;
}

IMU406 IMU406_Get_Yaw(void)
{
    return IMU406_YAW;
}

IMU406 IMU406_Get_AccX(void)
{
    return IMU406_ACC_X;
}

IMU406 IMU406_Get_AccY(void)
{
    return IMU406_ACC_Y;
}

IMU406 IMU406_Get_AccZ(void)
{
    return IMU406_ACC_Z;
}

IMU406 IMU406_Get_GyroX(void)
{
    return IMU406_GYRO_X;
}

IMU406 IMU406_Get_GyroY(void)
{
    return IMU406_GYRO_Y;
}

IMU406 IMU406_Get_GyroZ(void)
{
    return IMU406_GYRO_Z;
}