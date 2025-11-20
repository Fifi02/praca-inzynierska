// #include <cstring>
// #include <cstdlib>
// #include <ctime>
// #include <cstdio>

// extern "C" {
//     #include "freertos/FreeRTOS.h"
//     #include "freertos/task.h"
//     #include "freertos/event_groups.h"

//     #include "esp_wifi.h"
//     #include "esp_event.h"
//     #include "esp_log.h"
//     #include "nvs_flash.h"
//     #include "esp_netif.h"
//     #include "driver/gpio.h"
//     #include "esp_websocket_client.h"
// }

// static const char *TAG = "WS_APP";

// /* 🔧 KONFIGURACJA — ZMIEŃ NA SWOJE WiFi */
// #define WIFI_SSID      "Biustonosze_sciagac_prosze"
// #define WIFI_PASS      "Sexyhelena14"

// /* 🔧 Twój WebSocket server */
// #define WS_URL         "ws://158.180.49.234:8080"

// /* EventGroup do synchronizacji WiFi */
// static EventGroupHandle_t wifi_event_group;
// #define WIFI_CONNECTED_BIT BIT0


// #define pin_WIFI        GPIO_NUM_0
// #define pin_BLAD1       GPIO_NUM_16
// #define pin_BLAD2       GPIO_NUM_4
// #define SDA_HIGH_test   GPIO_NUM_22
// #define SCL_HIGH_test   GPIO_NUM_21
// #define sekcja1         GPIO_NUM_36   // INPUT ONLY
// #define sekcja2         GPIO_NUM_39   // INPUT ONLY
// #define sekcja3         GPIO_NUM_34   // INPUT ONLY
// #define sekcja4         GPIO_NUM_35   // INPUT ONLY
// #define sekcja5         GPIO_NUM_32
// #define pompa_230       GPIO_NUM_33
// #define SDA             GPIO_NUM_26
// #define SCL             GPIO_NUM_27

// /* ---------------------- WIFI EVENT HANDLER ---------------------- */
// extern "C" void wifi_event_handler(void* arg, esp_event_base_t event_base,
//                                    int32_t event_id, void* event_data)
// {
//     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {

//         ESP_LOGI(TAG, "WiFi started → connecting...");
//         esp_wifi_connect();
//     }
//     else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {

//         ESP_LOGW(TAG, "Disconnected → retrying...");
//         esp_wifi_connect();
//         xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
//     }
//     else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {

//         auto* event = (ip_event_got_ip_t*)event_data;
//         ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

//         xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
//     }
// }

// /* ---------------------- WIFI INIT ---------------------- */
// void wifi_init()
// {
//     ESP_LOGI(TAG, "Init NVS...");
//     nvs_flash_init();

//     ESP_LOGI(TAG, "Init NETIF + event loop...");
//     esp_netif_init();
//     esp_event_loop_create_default();
//     esp_netif_create_default_wifi_sta();

//     wifi_event_group = xEventGroupCreate();

//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     esp_wifi_init(&cfg);

//     wifi_config_t wifi_cfg = {};
//     strcpy((char*)wifi_cfg.sta.ssid, WIFI_SSID);
//     strcpy((char*)wifi_cfg.sta.password, WIFI_PASS);

//     esp_wifi_set_mode(WIFI_MODE_STA);
//     esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);

//     ESP_LOGI(TAG, "Registering WiFi event handlers...");

//     esp_event_handler_instance_register(
//         WIFI_EVENT, ESP_EVENT_ANY_ID,
//         &wifi_event_handler, NULL, NULL
//     );

//     esp_event_handler_instance_register(
//         IP_EVENT, IP_EVENT_STA_GOT_IP,
//         &wifi_event_handler, NULL, NULL
//     );

//     esp_wifi_start();

//     /* 🔥 Czekamy DO SKUTKU na połączenie WiFi */
//     ESP_LOGI(TAG, "Waiting for WiFi connection...");

//     xEventGroupWaitBits(
//         wifi_event_group,
//         WIFI_CONNECTED_BIT,
//         pdFALSE,
//         pdTRUE,
//         portMAX_DELAY    // ← czeka nieskończenie długo
//     );

//     ESP_LOGI(TAG, "WIFI CONNECTED ✔");
// }

// /* ---------------------- WEBSOCKET TASK ---------------------- */
// void websocket_task(void *param)
// {
//     esp_websocket_client_config_t ws_cfg = {};
//     ws_cfg.uri = WS_URL;

//     esp_websocket_client_handle_t client = esp_websocket_client_init(&ws_cfg);

//     ESP_LOGI(TAG, "Starting WebSocket client…");
//     esp_websocket_client_start(client);

//     /* Czekamy aż WebSocket się połączy */
//     while (!esp_websocket_client_is_connected(client)) {
//         ESP_LOGI(TAG, "Waiting for WS connection...");
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }

//     ESP_LOGI(TAG, "WebSocket connected ✔");

//     srand(time(NULL));

//     while (true) {
//         float temp = 20.0f + (rand() % 1000) / 100.0f;
//         char msg[32];
//         sprintf(msg, "%.2f", temp);

//         ESP_LOGI(TAG, "Sending temp: %s", msg);

//         esp_websocket_client_send_text(
//             client, msg, strlen(msg), portMAX_DELAY
//         );

//         vTaskDelay(pdMS_TO_TICKS(5000));  // co 5 sek
//     }
// }   

// static const gpio_num_t output_pins[] = {
//     pin_WIFI,
//     pin_BLAD1,
//     pin_BLAD2,
//     SDA_HIGH_test,
//     SCL_HIGH_test,
//     sekcja5,
//     pompa_230,
//     SDA,
//     SCL
// };

// void GPIO_init()
// {
//     ESP_LOGI("GPIO", "Inicjalizuję wszystkie piny wyjściowe...");
//     // 1. Konfiguracja wszystkich pinów jako wyjścia
//     gpio_config_t io_conf = {};
//     io_conf.intr_type = GPIO_INTR_DISABLE;
//     io_conf.mode = GPIO_MODE_OUTPUT;
//     io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
//     io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

//     // Tworzymy maskę bitową pinów
//     uint64_t pin_mask = 0;
//     for (int i = 0; i < sizeof(output_pins) / sizeof(output_pins[0]); i++) {
//         pin_mask |= (1ULL << output_pins[i]);
//     }

//     io_conf.pin_bit_mask = pin_mask;
//     gpio_config(&io_conf);
// }

// void GPIO_task(void *param)
// {
//        while (true) {
//         for (int i = 0; i < sizeof(output_pins) / sizeof(output_pins[0]); i++) {
//             gpio_set_level(output_pins[i], 1);
//         }
//         vTaskDelay(pdMS_TO_TICKS(3000));

//         // Zgaś wszystkie
//         for (int i = 0; i < sizeof(output_pins) / sizeof(output_pins[0]); i++) {
//             gpio_set_level(output_pins[i], 0);
//         }
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

// /* ---------------------- APP MAIN ---------------------- */
// extern "C" void app_main(void)
// {
//     ESP_LOGI(TAG, "app_main START");

//     /* Najpierw stabilne WiFi */
//     //wifi_init();
//     //GPIO_init();
//     /* Potem WebSocket task */
//     //xTaskCreate(websocket_task, "ws",4096, NULL, 5, NULL);
//     //xTaskCreate(GPIO_task, "ws",4096, NULL, 5, NULL);

// }

#include "common.h"
#include "lcd_i2c.h"
#include "task_buttons.h"
#include "task_leds.h"
#include "task_websocket.h"


#define BUTTON_GPIO   GPIO_NUM_19

static const char *TAG = "MAIN";

EventGroupHandle_t  xISREventGroup = xEventGroupCreate();
EventGroupHandle_t wifi_event_group = xEventGroupCreate();

uint16_t section_num = 1;

// I2C Scanner Function
void i2c_scanner() {
    ESP_LOGI(TAG, "Scanning I2C bus...");
    uint8_t count = 0;
    
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found device at address: 0x%02X", addr);
            count++;
        }
    }
    
    if (count == 0) {
        ESP_LOGW(TAG, "No I2C devices found!");
    } else {
        ESP_LOGI(TAG, "Found %d device(s)", count);
    }
}




extern "C" void app_main() {
    ESP_LOGI(TAG, "Starting LCD 20x4 example");
    // Initialize I2C
    ESP_ERROR_CHECK(lcd_i2c_init());
    ESP_LOGI(TAG, "I2C initialized");
    
    // Scan for I2C devices
    //i2c_scanner();
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    xTaskCreate(task_buttons, "Buttons", 2048, NULL, 10, NULL);
    xTaskCreate(task_lcd, "LCD", 2048, NULL, 10, NULL);
    xTaskCreate(task_leds, "LEDS", 2048, NULL, 10, NULL);
    xTaskCreate(websocket_task, "WebSocket",4096, NULL, 5, NULL);
    
}