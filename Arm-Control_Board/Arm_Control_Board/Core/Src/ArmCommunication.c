//
// Created by s on 26-5-31.
//

#include "ArmCommunication.h"

uint32_t move_steps = 0;
bool isClose = false;
bool getSteps = false;

void Arm_Serial_Init(void) {
    MX_USART1_UART_Init();

    HAL_UART_Receive_IT(&huart1, &arm_rxByte, 1);
}

void Arm_Receive_ISR(void) {
    if (arm_rxIndex == 0U) {
        if (arm_rxByte != ARM_RECIEVE_MESSAGE_HEAD) {
            goto _next;
        }
    }

    arm_rxBuf[arm_rxIndex++] = arm_rxByte;

    if (arm_rxIndex >= ARM_RECEIVE_MESSGAE_LEN) {
        Arm_Receive_ParseFrame(arm_rxBuf);
        arm_rxIndex = 0U;
    }

    _next:
    HAL_UART_Receive_IT(&huart1, &arm_rxByte, 1);
}

static void Arm_Receive_ParseFrame(const uint8_t *frame) {
    uint8_t calc_checksum = 0;
    for (int i = 0; i < ARM_RECEIVE_MESSGAE_LEN - 1; i++) {
        calc_checksum += frame[i];
    }
    if (calc_checksum != frame[ARM_RECEIVE_MESSGAE_LEN - 1]) {
        return;
    }

    const uint8_t *pData = (const uint8_t *)&frame[1];
    isClose = (bool) pData[0];
    uint32_t combined = (pData[1] << 16) | (pData[2] << 8) | pData[3];

    move_steps = combined;
    getSteps = true;
}

void Arm_Transit(void) {

}
