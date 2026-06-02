//
// Created by s on 26-4-12.
//

#ifndef ARM_CONTROL_BOARD_ARM_H
#define ARM_CONTROL_BOARD_ARM_H

#include "42_stepper.h"
#include "servo.h"
#include "ArmCommunication.h"

extern Servo mechanical_claw_servo;
extern Servo camera_holder_servo;

typedef struct {
    float x;
    float z;
};

void Arm_Init(void);
void Direction_Init(Stepper *stepper);

void Move_to_Get(void);

void Parase_Camera_Coordinate(void);

#endif //ARM_CONTROL_BOARD_ARM_H
