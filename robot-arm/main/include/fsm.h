#ifndef FSM_H
#define FSM_H

#include <stdbool.h>
#include "arm.h"

typedef enum {
    IDLE,
    CARRYING
} state_t;
extern volatile state_t state;

typedef struct {
    bool reset;
    bool mode;
    bool is_tennis_ball;
    bool in_range;
    bool pick;
    bool launch;
} flags_t;
extern volatile flags_t flags;

void change_state();

#endif