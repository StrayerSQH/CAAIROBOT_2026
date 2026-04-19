//
// Created by s on 26-1-22.
//

#include "usart.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef INC_2026_AICUP_STM32F407VET6_BLUETOOTH_H
#define INC_2026_AICUP_STM32F407VET6_BLUETOOTH_H

typedef uint8_t JOYSTICK;

extern volatile JOYSTICK JOYSTICK_FB;
extern volatile JOYSTICK JOYSTICK_LR;
extern volatile JOYSTICK JOYSTICK_Z;

#define Bluetooth_FRAME_LEN     4U
#define Bluetooth_FRAME_HEAD    0x48

static uint8_t  bluetooth_rxByte = 0;
static uint8_t  bluetooth_rxBuf[Bluetooth_FRAME_LEN];
static uint8_t bluetooth_rxIndex = 0;

// 函数声明
void Bluetooth_Init(void); //初始化蓝牙
void Bluetooth_Rx_ISR(void);
static void Bluetooth_ParseFrame(const uint8_t *frame);

// 内联函数完整定义
//static inline JOYSTICK Bluetooth_DecodeBytes(const uint8_t *p);

#endif //INC_2026_AICUP_STM32F407VET6_BLUETOOTH_H