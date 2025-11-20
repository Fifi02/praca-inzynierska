#include "task_websocket.h"
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cstdio>


static const char *TAG = "WS_APP";
extern EventGroupHandle_t wifi_event_group;
/* ---------------------- WIFI EVENT HANDLER ---------------------- */
extern "C" void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                   int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {

        ESP_LOGI(TAG, "WiFi started → connecting...");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {

        ESP_LOGW(TAG, "Disconnected → retrying...");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {

        auto* event = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* ---------------------- WIFI INIT ---------------------- */
void wifi_init()
{
    ESP_LOGI(TAG, "Init NVS...");
    nvs_flash_init();

    ESP_LOGI(TAG, "Init NETIF + event loop...");
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_event_group = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_cfg = {};
    strcpy((char*)wifi_cfg.sta.ssid, WIFI_SSID);
    strcpy((char*)wifi_cfg.sta.password, WIFI_PASS);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);

    ESP_LOGI(TAG, "Registering WiFi event handlers...");

    esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID,
        &wifi_event_handler, NULL, NULL
    );

    esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP,
        &wifi_event_handler, NULL, NULL
    );

    esp_wifi_start();

    /* 🔥 Czekamy DO SKUTKU na połączenie WiFi */
    ESP_LOGI(TAG, "Waiting for WiFi connection...");

    xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY    // ← czeka nieskończenie długo
    );

    ESP_LOGI(TAG, "WIFI CONNECTED ✔");
}


static void websocket_event_handler(void *handler_args,
                                    esp_event_base_t base,
                                    int32_t event_id,
                                    void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;

    switch(event_id) {

        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI("WS", "Connected to WS server");
            break;

        case WEBSOCKET_EVENT_DATA:
            ESP_LOGI("WS", "Received data (len=%d): %.*s",
                     data->data_len,
                     data->data_len,
                     (char*)data->data_ptr);

            // 🔥 tutaj obsługujemy komendy
            if (strncmp((char*)data->data_ptr, "ON", 2) == 0) {
                ESP_LOGI("WS", "Received command: ON");
                xEventGroupSetBits(xISREventGroup, BTN0_PUSHED);
                // sterowanie pinem tutaj
            }

            if (strncmp((char*)data->data_ptr, "OFF", 3) == 0) {
                ESP_LOGI("WS", "Received command: OFF");
                 xEventGroupSetBits(xISREventGroup, BTN3_PUSHED);
                // sterowanie pinem tutaj
            }

            break;

        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGW("WS", "Disconnected from WS server");
            break;

        default:
            ESP_LOGI("WS", "Other WS event: %d", event_id);
            break;
    }
}

/* ---------------------- WEBSOCKET TASK ---------------------- */
void websocket_task(void *param)
{
    wifi_init();
    esp_websocket_client_config_t ws_cfg = {};
    ws_cfg.uri = WS_URL;

    esp_websocket_client_handle_t client = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(
    client,
    WEBSOCKET_EVENT_ANY,
    websocket_event_handler,
    NULL
    );
    ESP_LOGI(TAG, "Starting WebSocket client…");
    esp_websocket_client_start(client);

    /* Czekamy aż WebSocket się połączy */
    while (!esp_websocket_client_is_connected(client)) {
        ESP_LOGI(TAG, "Waiting for WS connection...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    ESP_LOGI(TAG, "WebSocket connected ✔");

    srand(time(NULL));

    while (true) {
        float temp = 20.0f + (rand() % 1000) / 100.0f;
        char msg[32];
        sprintf(msg, "%.2f", temp);

        ESP_LOGI(TAG, "Sending temp: %s", msg);

        esp_websocket_client_send_text(
            client, msg, strlen(msg), portMAX_DELAY
        );

        vTaskDelay(pdMS_TO_TICKS(5000));  // co 5 sek
    }
}   
