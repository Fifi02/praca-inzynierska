#include "common.h"
extern "C" {
    #include "esp_wifi.h"
    #include "nvs_flash.h"
    #include "esp_netif.h"
    #include "esp_websocket_client.h"
}
#define WIFI_CONNECTED_BIT BIT0
/* 🔧 KONFIGURACJA — ZMIEŃ NA SWOJE WiFi */
#define WIFI_SSID      "Biustonosze_sciagac_prosze"
#define WIFI_PASS      "Sexyhelena14"

/* 🔧 Twój WebSocket server */
#define WS_URL         "ws://158.180.49.234:8080/esp"
void websocket_task(void *param);
extern esp_websocket_client_handle_t client;