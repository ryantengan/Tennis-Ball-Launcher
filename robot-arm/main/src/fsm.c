#include "fsm.h"

volatile state_t state = IDLE;
volatile flags_t flags = {0};

void change_state()
{
    if (flags.reset)
    {
        state = IDLE;
        flags.mode = true;
    }
    else
    {
        switch (state)
        {
            case IDLE:
                if (flags.pick /*&& flags.is_tennis_ball*/)
                {
                    pick();
                    flags.pick = false;

                    state = CARRYING;
                }
                break;
            case CARRYING:
                if (flags.launch)
                {
                    launch();
                    flags.launch = false;

                    state = IDLE;
                }
                break;
        }
    }
}
