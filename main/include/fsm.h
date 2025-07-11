#ifndef FSM_H
#define FSM_H

#include <stdbool.h>

typedef enum {
    DRIVE,
    PICKING,
    LAUNCHING
} state_t;
extern volatile state_t state;

extern volatile bool reset,
                    mode, // 0 = autonomous, 1 = manual
                    drive_flag,
                    is_tennis_ball,
                    in_range,
                    pickup,
                    ready_l,
                    launch;

void change_state();

#endif