#include "task_leds.h"




static const char *TAG = "LEDS";

enum{
    LED_ERR1,
    LED_ERR2,
    LED_WIFI
};

int led_pin[LED_NUM] = {LED_ERR1_PIN,LED_ERR2_PIN,LED_WIFI_PIN};
bool led_state[LED_NUM]={0,0,0};

void leds_init(EventGroupHandle_t ev_group)
{
        gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_ERR1_PIN) | (1ULL << LED_ERR2_PIN) | (1ULL << LED_WIFI_PIN),
        .mode = GPIO_MODE_OUTPUT,              // ustaw jako wyjście
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE         // brak przerwań dla wyjścia
    };

    gpio_config(&io_conf);
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}


void toggle_led(int led_number)
{
    //printf("State:%d\n",led_state[led_pin[led_number]]);
    //printf("led_number:%d\n",led_number);
    //printf("led_number:%d\n",led_pin[led_number]);
    led_state[led_number]=!led_state[led_number];
    gpio_set_level(led_pin[led_number],led_state[led_number]);
}



void task_leds(__unused void *params)
{ 
    ESP_LOGI(TAG, "task_leds starting");
    leds_init(xISREventGroup); 
    while (true) 
    { 
        EventBits_t evBits = xEventGroupWaitBits( xISREventGroup, BTN0_PUSHED | BTN1_PUSHED | BTN2_PUSHED | BTN3_PUSHED, pdTRUE,pdFALSE,portMAX_DELAY);
        if (evBits & BTN0_PUSHED)
        {   
            printf("LED1\n");
            toggle_led(LED_ERR1);
        }
        if (evBits & BTN1_PUSHED)
        {   
            printf("LED2\n");
            toggle_led(LED_ERR2);
        }
        if (evBits & BTN2_PUSHED)
        {   
            printf("LED3\n");
            toggle_led(LED_WIFI);
        }
        if (evBits & BTN3_PUSHED)
        {   
            printf("LED4\n");
            toggle_led(LED_ERR1);
            toggle_led(LED_ERR2);
            toggle_led(LED_WIFI);
        }
    }
}
