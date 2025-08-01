#include "arm.h"

static int servo_angles[] = {
    [BASE] = 90,
    [ARM] = 100,
    [FOREARM] = 180,
    [GRIPPERS] = GRIPPERS_CLOSED
};

static int to_pulse(int angle)
{
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    return (int)(90 + ((angle / 180.0) * 400));
}

void set_angle(joint_t joint, int angle)
{
    setPWM(joint, 0, to_pulse(angle));
    servo_angles[joint] = angle;
}

void rotate_servo(joint_t joint, int new_angle)
{
    if (servo_angles[joint] < new_angle)
    {
        for (int angle = servo_angles[joint]; angle <= new_angle; angle++)
        {
            set_angle(joint, angle);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    else
    {
        for (int angle = servo_angles[joint]; angle >= new_angle; angle--)
        {
            set_angle(joint, angle);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void init_arm(void)
{
    for (joint_t joint = BASE; joint <= GRIPPERS; joint++)
    {
        set_angle(joint, servo_angles[joint]);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void toggle_claw(void)
{
    if (servo_angles[GRIPPERS] == GRIPPERS_OPEN)
        rotate_servo(GRIPPERS, GRIPPERS_CLOSED);
    else
        rotate_servo(GRIPPERS, GRIPPERS_OPEN);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void pickup(void)
{
    if (servo_angles[GRIPPERS] == GRIPPERS_CLOSED)
        toggle_claw();
    rotate_servo(FOREARM, 65);
    rotate_servo(ARM, 0);

    toggle_claw(); // Grab object
    vTaskDelay(pdMS_TO_TICKS(1000));

    rotate_servo(ARM, 100);
    rotate_servo(FOREARM, 145);
}