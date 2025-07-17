#include "arm.h"

volatile bool claw_state = 0;

void toggle_claw(void)
{
    // Close claw
    if (claw_state)
    {
        setPWM(0, 0, 430);
        printf("Closed\n"); fflush(stdout);
        claw_state = false;
    }
    // Open claw
    else
    {
        setPWM(0, 0, 320);
        printf("Open\n"); fflush(stdout);
        claw_state = true;
    }
}