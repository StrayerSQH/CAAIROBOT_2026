//
// Created by s on 26-4-12.
//
#include "Arm.h"

Servo mechanical_claw_servo = {0};
Servo camera_holder_servo = {0};

Stepper steppers[3];

void Arm_Init(void) {
    Stepper_Init(&steppers[0], 1, 5000, 2000);
    Stepper_Init(&steppers[1], 2, 5000, 2000);

    Direction_Init(&steppers[0]); // x方向伸缩校准
    //Direction_Init(&steppers[1]); // z方向上下校准

    // 机械爪测试
    Servo_Set_Angel(&mechanical_claw_servo, 0);
    HAL_Delay(500);
    Servo_Set_Angel(&mechanical_claw_servo, 90);
    HAL_Delay(500);
    Servo_Set_Angel(&mechanical_claw_servo, 0);
}

void Direction_Init(Stepper *stepper) {
    stepper->isStall = false;
    stepper->isPosition = false;

    uint8_t direction;
    switch (stepper->stepper_can_id) {
        case 1:
            direction = 0;
            break;
        case 2:
            direction = 1;
            break;
        default:
            direction = 0;

    }

    // 如果电机没有发生堵转，则转动一圈
    while (!stepper->isStall) {
        Relative_Position_Set_Command(stepper->stepper_can_id,
                                      direction,
                                      10,
                                      5500,
                                      51200);
        Send_Message_to_Stepper(set_relative_position_command,
                                sizeof(set_relative_position_command));
        HAL_Delay(1000);

        // 如果电机没有到位则一直在循环里
        while (stepper->isPosition) {
            Position_Question_Command(stepper->stepper_can_id);
            Send_Message_to_Stepper(position_question_commmand,
                                    sizeof(position_question_commmand));
            HAL_Delay(10);
        }
        stepper->isPosition = false;

        Stall_Question_Command(stepper->stepper_can_id);
        Send_Message_to_Stepper(stall_question_command,
                                sizeof(stall_question_command));
    }

    for (int i = 0; i < 3; i++) { //反复发送停转指令，确保电机收到
        Stop_Command(stepper->stepper_can_id);
        HAL_Delay(10);
    }

    for (int i = 0; i < 3; i++) { //反复发送清除状态指令，确保电机收到
        Clear_State_Command(stepper->stepper_can_id);
        HAL_Delay(10);
    }

    stepper->isStall = false;
}

void Move_to_Get(void) {
    Servo_Set_Angel(&mechanical_claw_servo, 60);
    Relative_Position_Set_Command(steppers[0].stepper_can_id,
                                  0,
                                  20,
                                  7000,
                                  move_steps);
    Send_Message_to_Stepper(set_relative_position_command,
                            sizeof(set_relative_position_command));
    HAL_Delay(2000);
}

void Move_to_Origin(void) {
    Relative_Position_Set_Command(steppers[0].stepper_can_id,
                                  1,
                                  20,
                                  7000,
                                  move_steps);
    Send_Message_to_Stepper(set_relative_position_command,
                            sizeof(set_relative_position_command));
    HAL_Delay(2000);
}