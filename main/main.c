#include <stdio.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sdkconfig.h"
#include "pca9685.h"

// Servo motor control
static void I2C_Init(void);
static void PCA9685_Init(void);
void task_PCA9685();

// Bluetooth communication
uint8_t ble_addr_type;

static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int ble_gap_event(struct ble_gap_event *event, void *arg);
void ble_app_advertise(void);
void ble_app_on_sync(void);
void host_task(void *param);

static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(0x180),                 // Define UUID for device type
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(0xFEF4),           // Define UUID for reading
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = device_read},
         {.uuid = BLE_UUID16_DECLARE(0xDEAD),           // Define UUID for writing
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {0}}},
    {0}};

void app_main()
{
    // // Initializing peripherals
    // I2C_Init();
    // PCA9685_Init();

    // xTaskCreate(task_PCA9685, "task_PCA9685", 1024 * 2, NULL, 10, NULL);

    nvs_flash_init();                          // 1 - Initialize NVS flash using
    nimble_port_init();                        // 3 - Initialize the host stack
    
    ble_svc_gap_device_name_set("BLE-Server"); // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
    ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application

    nimble_port_freertos_init(host_task);      // 6 - Run the thread
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

// Write data to ESP32 defined as server
static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    char * data = (char *)ctxt->om->om_data;
    printf("%d\n",strcmp(data, (char *)"LIGHT ON")==0);
    if (strcmp(data, (char *)"LIGHT ON\0")==0)
    {
       printf("LIGHT ON\n");
    }
    else if (strcmp(data, (char *)"LIGHT OFF\0")==0)
    {
        printf("LIGHT OFF\n");
    }
    else if (strcmp(data, (char *)"FAN ON\0")==0)
    {
        printf("FAN ON\n");
    }
    else if (strcmp(data, (char *)"FAN OFF\0")==0)
    {
        printf("FAN OFF\n");
    }
    else{
        printf("Data from the client: %.*s\n", ctxt->om->om_len, ctxt->om->om_data);
    }
    
    return 0;
}

// Read data from ESP32 defined as server
static int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    os_mbuf_append(ctxt->om, "Data from the server", strlen("Data from the server"));
    return 0;
}

// BLE event handling
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    // Advertise if connected
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status != 0)
        {
            ble_app_advertise();
        }
        break;
    // Advertise again after completion of the event
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT DISCONNECTED");
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    default:
        break;
    }
    return 0;
}

// Define the BLE connection
void ble_app_advertise(void)
{
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name(); // Read the BLE device name
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

// The application
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type); // Determines the best address type automatically
    ble_app_advertise();                     // Define the BLE connection
}

// The infinite task
void host_task(void *param)
{
    nimble_port_run(); // This function will return only when nimble_port_stop() is executed
}