#ifndef ARM_H
#define ARM_H

#include "pca9685.h"
#include "driver/ledc.h"

#define GRIPPERS_OPEN 65
#define GRIPPERS_CLOSED 0

typedef enum {
    BASE,
    ARM,
    FOREARM,
    GRIPPERS
} joint_t;

void set_angle(joint_t joint, int angle);
void rotate_servo(joint_t, int angle);
void init_arm(void);
void toggle_claw(void);
void pickup(void);

#endif