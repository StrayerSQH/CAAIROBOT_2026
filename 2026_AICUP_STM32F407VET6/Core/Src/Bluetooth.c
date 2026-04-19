//
// Created by s on 26-1-22.
//
#include "Bluetooth.h"

volatile JOYSTICK JOYSTICK_FB = 0;
volatile JOYSTICK JOYSTICK_LR = 0;
volatile JOYSTICK JOYSTICK_Z = false; //false禁止左右平移

void Bluetooth_Init(void) {
    MX_USART2_UART_Init();
    HAL_UART_Receive_IT(&huart2, &bluetooth_rxByte, 1);
}

void Bluetooth_Rx_ISR() {
    if (bluetooth_rxIndex == 0U) {
        if (bluetooth_rxByte != Bluetooth_FRAME_HEAD) {
            goto _next;
        }
    }

    bluetooth_rxBuf[bluetooth_rxIndex++] = bluetooth_rxByte;

    if (bluetooth_rxIndex >= Bluetooth_FRAME_LEN) {
        Bluetooth_ParseFrame(bluetooth_rxBuf);
        bluetooth_rxIndex = 0U;
    }

    _next:
    HAL_UART_Receive_IT(&huart2, &bluetooth_rxByte, 1);
}

static void Bluetooth_ParseFrame(const uint8_t *frame) {
    JOYSTICK_FB = frame[1];
    JOYSTICK_LR = frame[2];
    JOYSTICK_Z = frame[3];
}
//
//static inline JOYSTICK Bluetooth_DecodeBytes(const uint8_t *p) {
//    return p[0];
//}