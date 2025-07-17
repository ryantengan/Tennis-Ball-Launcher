#ifndef ARM_H
#define ARM_H

#include "pca9685.h"
#include "driver/ledc.h"
#include "stdbool.h"

extern volatile bool claw_state; // false = open, true = closed

void toggle_claw(void);

#endif