#include "fsm.h"

volatile state_t state = DRIVE;
volatile bool reset,
            mode,
            drive_flag,
            is_tennis_ball,
            in_range,
            pickup,
            ready_l,
            launch;

void change_state()
{
    if (reset)
    {
        state = DRIVE;
        mode = true;
        drive_flag = true;
    }
    else
    {
        switch(state)
        {
            case DRIVE:
                if (pickup && is_tennis_ball && in_range)
                {
                    drive_flag = false;
                    state = PICKING;
                }
                if (launch)
                {
                    ready_l = false;
                    drive_flag = false;
                    state = LAUNCHING;
                }
                break;
            case PICKING:
                if (!pickup)
                {
                    ready_l = true;
                    drive_flag = true;
                    state = DRIVE;
                }
                break;
            case LAUNCHING:
                if (!launch)
                {
                    drive_flag = true;
                    state = DRIVE;
                }
                break;
        }
    }
}