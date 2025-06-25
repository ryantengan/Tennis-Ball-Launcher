#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "pca9685.h"

static void I2C_Init(void);
static void PCA9685_Init(void);

void task_PCA9685();

void app_main()
{
    // Initializing peripherals
    I2C_Init();
    PCA9685_Init();

    xTaskCreate(task_PCA9685, "task_PCA9685", 1024 * 2, NULL, 10, NULL);
}

void task_PCA9685()
{
    while (1)
    {
        setPWM(0, 0, 100);
        printf("0 degrees\n"); fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(1000));

        setPWM(0, 0, 315);
        printf("90 degrees\n"); fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(1000));

        setPWM(0, 0, 515);
        printf("180 degrees\n"); fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void PCA9685_Init(void)
{
    set_pca9685_adress(0x40);
    resetPCA9685();
    setFrequencyPCA9685(50);
    turnAllOff();
}

static void I2C_Init(void)
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = 5;
    conf.scl_io_num = 4;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    
    int i2c_master_port = I2C_NUM_0;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}