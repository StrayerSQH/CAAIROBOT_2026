//
// Created by s on 26-4-11.
//

#ifndef ARM_CONTROL_BOARD_42_STEPPER_H
#define ARM_CONTROL_BOARD_42_STEPPER_H

#include <stdint-gcc.h>
#include <stdbool.h>
#include <stddef.h>
#include <can.h>

#define FRAME_HEAD 0xC5
#define FRAME_TAIL 0x5C

typedef union {
    float f;
    uint8_t bytes[4];
} FloatBytes;

typedef struct {
    uint16_t stepper_can_id;
    float current_position;
    float speed;
    float current_I;
    bool isStall;
    bool isPosition;
} Stepper;

extern Stepper steppers[3];

extern CAN_TxHeaderTypeDef stepper_tx_message;
extern uint8_t clear_state_command[5];
extern uint8_t stop_command[5];
extern uint8_t stall_question_command[5];
extern uint8_t position_question_commmand[5];
extern uint8_t set_current_command[8];
extern uint8_t set_relative_position_command[13];
extern uint8_t set_absolute_position_command[13];
extern uint8_t read_command[6];

void Stepper_Init(Stepper *stepper, uint16_t stepper_can_id, uint16_t speed, uint16_t current_I);

void Clear_State_Command(uint8_t stepper_can_id);
void Stop_Command(uint8_t stepper_can_id);
void Stall_Question_Command(uint8_t stepper_can_id);
void Position_Question_Command(uint8_t stepper_can_id);
void Current_Set_Command(uint8_t direction, uint16_t current_I, uint8_t stepper_can_id);
void Relative_Position_Set_Command(uint8_t stepper_can_id,
                                   uint8_t direction,
                                   uint16_t accelerator,
                                   uint16_t speed,
                                   uint32_t relative_steps);

void Absolute_Position_Set_Command(uint8_t stepper_can_id,
                                   uint8_t direction,
                                   uint16_t accelerator,
                                   uint16_t speed,
                                   uint32_t absolute_steps);

static uint8_t Checksum_Generator(const uint8_t *data, size_t len);

void Send_Message_to_Stepper(uint8_t *data, uint8_t len);
void Read_Message_from_Stepper(uint8_t *data);


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

#endif //ARM_CONTROL_BOARD_42_STEPPER_H
