//
// Created by s on 26-5-31.
//

#ifndef ARM_CONTROL_BOARD_ARMCOMMUNICATION_H
#define ARM_CONTROL_BOARD_ARMCOMMUNICATION_H

#include <stdbool.h>
#include "usart.h"

#define ARM_RECEIVE_MESSGAE_LEN     6U
/* 第一位帧头
 * 第二位机械爪张开闭合
 * 第三、四位摄像头坐标
 * 第五位校验和
 * */
#define ARM_RECIEVE_MESSAGE_HEAD    0x21

#define ARM_TRANSIT_MESSGAE_LEN     4U
#define ARM_TRANSIT_MESSAGE_HEAD    0x22

static uint8_t arm_rxByte = 0;
static uint8_t arm_rxBuf[ARM_RECEIVE_MESSGAE_LEN];
static uint8_t arm_rxIndex = 0;

extern uint32_t move_steps;
extern bool isClose;
extern bool getSteps;

void Arm_Serial_Init(void);

void Arm_Receive_ISR(void);
static void Arm_Receive_ParseFrame(const uint8_t *frame);

void Arm_Transit(void);

#endif //ARM_CONTROL_BOARD_ARMCOMMUNICATION_H
