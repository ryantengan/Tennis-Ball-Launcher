#include <stdio.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "driver/ledc.h"
#include "pca9685.h"
#include "fsm.h"
#include "ble_server.h"

// Servo motor control
static void I2C_Init(void);
static void PCA9685_Init(void);
static void PWM_Init(void);

// Bluetooth communication
static void BLE_Server_Init(void);

void app_main()
{
    // Initializations
    I2C_Init();
    PCA9685_Init();
    PWM_Init();
    BLE_Server_Init();

    nimble_port_freertos_init(host_task);
}

/* SERVO MOTOR CONTROL */

static void PCA9685_Init(void)
{
    set_pca9685_adress(0x40);
    resetPCA9685();
    setFrequencyPCA9685(50);
    turnAllOff();
}

static void I2C_Init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 21,
        .scl_io_num = 22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };

    int i2c_master_port = I2C_NUM_0;

    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
    i2c_set_pin(i2c_master_port, conf.sda_io_num, conf.scl_io_num, conf.sda_pullup_en, conf.scl_pullup_en, conf.mode);
}

static void PWM_Init(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 50,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = 13,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);
}

/* BLUETOOTH LOW ENERGY */

static void BLE_Server_Init(void)
{
    nvs_flash_init();
    nimble_port_init();

    ble_svc_gap_device_name_set("Tennis Bot");
    ble_svc_gap_init();
    ble_svc_gatt_init();

    static ble_uuid16_t read_uuid = BLE_UUID16_INIT(0xFEF4);
    static ble_uuid16_t write_uuid = BLE_UUID16_INIT(0xDEAD);
    static ble_uuid16_t service_uuid = BLE_UUID16_INIT(0x0180);

    static struct ble_gatt_chr_def characteristics[] = {
        {
            .uuid = (ble_uuid_t *)&read_uuid,
            .flags = BLE_GATT_CHR_F_READ,
            .access_cb = device_read
        },
        {
            .uuid = (ble_uuid_t *)&write_uuid,
            .flags = BLE_GATT_CHR_F_WRITE,
            .access_cb = device_write
        },
        { 0 }
    };

    static const struct ble_gatt_svc_def gatt_svcs[] = {
        {
            .type = BLE_GATT_SVC_TYPE_PRIMARY,
            .uuid = (ble_uuid_t *)&service_uuid,
            .characteristics = characteristics
        },
        { 0 }
    };

    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);
    ble_hs_cfg.sync_cb = ble_app_on_sync;
}

void host_task(void *param)
{
    nimble_port_run();
}