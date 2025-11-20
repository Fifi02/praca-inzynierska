idf_component_register(
    SRCS "main.cpp" "lcd_i2c.cpp" "common.c" "task_buttons.c" "task_leds.c" "task_websocket.cpp" "task_sections.cpp" 
    INCLUDE_DIRS "."
    REQUIRES esp_websocket_client esp_wifi esp_netif nvs_flash esp_event  driver
    PRIV_REQUIRES esp_driver_gpio 
)