//
// Created by s on 26-4-11.
//
#include "42_stepper.h"

CAN_TxHeaderTypeDef stepper_tx_message;

uint8_t clear_state_command[5];
uint8_t stop_command[5];
uint8_t stall_question_command[5];
uint8_t position_question_commmand[5];
uint8_t set_current_command[8];
uint8_t set_relative_position_command[13];
uint8_t set_absolute_position_command[13];
uint8_t read_command[6];

void Stepper_Init(Stepper *stepper, uint16_t stepper_can_id, uint16_t speed, uint16_t current_I) {
    stepper->stepper_can_id = stepper_can_id;
    stepper->speed = speed;
    stepper->current_I = current_I;
    stepper->isStall = false;
    stepper->isPosition = false;
}

void Clear_State_Command(uint8_t stepper_can_id) {
    clear_state_command[0] = FRAME_HEAD;
    clear_state_command[1] = 0xFF & stepper_can_id;
    clear_state_command[2] = 0xFC;
    clear_state_command[3] =  Checksum_Generator(clear_state_command, 3);
    clear_state_command[4] = FRAME_TAIL;
}

void Stop_Command(uint8_t stepper_can_id) {
    stop_command[0] = FRAME_HEAD;
    stop_command[1] = 0xFF & stepper_can_id;
    stop_command[2] = 0xFC;
    stop_command[3] =  Checksum_Generator(stop_command, 3);
    stop_command[4] = FRAME_TAIL;
}


void Stall_Question_Command(uint8_t stepper_can_id) {
    stall_question_command[0] = FRAME_HEAD;
    stall_question_command[1] = 0xFF & stepper_can_id;
    stall_question_command[2] = 0x2D;
    stall_question_command[3] =  Checksum_Generator(stall_question_command, 3);
    stall_question_command[4] = FRAME_TAIL;
}

void Position_Question_Command(uint8_t stepper_can_id) {
    position_question_commmand[0] = FRAME_HEAD;
    position_question_commmand[1] = 0xFF & stepper_can_id;
    position_question_commmand[2] = 0x30;
    position_question_commmand[3] =  Checksum_Generator(position_question_commmand, 3);
    position_question_commmand[4] = FRAME_TAIL;
}

void Current_Set_Command(uint8_t direction, uint16_t current_I, uint8_t stepper_can_id) {
    set_current_command[0] = FRAME_HEAD;
    set_current_command[1] = 0xFF & stepper_can_id;
    set_current_command[2] = 0xF0;
    set_current_command[3] = 0xFF & direction;
    set_current_command[4] = (current_I >> 8) & 0xFF;
    set_current_command[5] = current_I & 0xFF;
    set_current_command[6] = Checksum_Generator(set_current_command, 6);
    set_current_command[7] = FRAME_TAIL;
}

void Relative_Position_Set_Command(uint8_t stepper_can_id, uint8_t direction, uint16_t accelerator, uint16_t speed, uint32_t relative_steps) {
    set_relative_position_command[0] = FRAME_HEAD;
    set_relative_position_command[1] = 0xFF & stepper_can_id;
    set_relative_position_command[2] = 0xF3;
    set_relative_position_command[3] = 0xFF & direction;
    set_relative_position_command[4] = 0xFF & accelerator;
    set_relative_position_command[5] = (speed >> 8) & 0xFF;
    set_relative_position_command[6] = speed & 0xFF;
    set_relative_position_command[7] = (relative_steps >> 24) & 0xFF;
    set_relative_position_command[8] = (relative_steps >> 16) & 0xFF;
    set_relative_position_command[9] = (relative_steps >> 8) & 0xFF;
    set_relative_position_command[10] = relative_steps & 0xFF;
    set_relative_position_command[11] = Checksum_Generator(set_relative_position_command, 11);
    set_relative_position_command[12] = FRAME_TAIL;

}

void Absolute_Position_Set_Command(uint8_t stepper_can_id, uint8_t direction, uint16_t accelerator, uint16_t speed, uint32_t absolute_steps) {
    set_absolute_position_command[0] = FRAME_HEAD;
    set_absolute_position_command[1] = 0xFF & stepper_can_id;
    set_absolute_position_command[2] = 0xF2;
    set_absolute_position_command[3] = 0xFF & direction;
    set_absolute_position_command[4] = 0xFF & accelerator;
    set_absolute_position_command[5] = (speed >> 8) & 0xFF;
    set_absolute_position_command[6] = speed & 0xFF;
    set_absolute_position_command[7] = (absolute_steps >> 24) & 0xFF;
    set_absolute_position_command[8] = (absolute_steps >> 16) & 0xFF;
    set_absolute_position_command[9] = (absolute_steps >> 8) & 0xFF;
    set_absolute_position_command[10] = absolute_steps & 0xFF;
    set_absolute_position_command[11] = Checksum_Generator(set_absolute_position_command, 11);
    set_absolute_position_command[12] = FRAME_TAIL;
}

static uint8_t Checksum_Generator(const uint8_t *data, size_t len) {
    uint8_t sum = 0;

    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

void Send_Message_to_Stepper(uint8_t *data, uint8_t len) {
    uint32_t wait_time;

    uint32_t  send_mailbox;
    stepper_tx_message.StdId = 0x1000;
    stepper_tx_message.ExtId = 0x1000;
    stepper_tx_message.IDE = CAN_ID_EXT;
    stepper_tx_message.RTR = CAN_RTR_DATA;
    stepper_tx_message.TransmitGlobalTime = DISABLE;

    if (len > 8) {
        uint16_t offset = 0;
        uint8_t send_len;

        while (offset < len)
        {
            send_len = (len - offset >= 8) ? 8 : (len - offset);
            stepper_tx_message.DLC = send_len;

            if (HAL_CAN_AddTxMessage(&hcan,
                                     &stepper_tx_message,
                                     data + offset,
                                     &send_mailbox) != HAL_OK) {
                return;
            }

            wait_time = 0xFFFF;
            while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3)
            {
                if (--wait_time == 0) return;
            }

            offset += send_len;
        }
    }
    else {
        stepper_tx_message.DLC = len;
        if (HAL_CAN_AddTxMessage(&hcan,
                                 &stepper_tx_message,
                                 data,
                                 &send_mailbox) != HAL_OK) {
            return;
        }

        wait_time = 0xFFFF;
        while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3)
        {
            if (--wait_time == 0) return;
        }
    }
}

void Read_Message_from_Stepper(uint8_t *data) {
    switch (data[3]) {
        case 0x30: // 电机到位指令查询
            steppers[data[2] - 1].isPosition = (bool) data[5];
            break;
        case 0x2D: // 电机堵转指令查询
            steppers[data[2] - 1].isStall = (bool) data[5];
            break;
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef rx_header;
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, read_command);
    Read_Message_from_Stepper(read_command);
}