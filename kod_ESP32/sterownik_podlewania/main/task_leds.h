#pragma once
#include "common.h"
#define LED_ERR1_PIN    16    
#define LED_ERR2_PIN    4
#define LED_WIFI_PIN    0
#define LED_NUM 3

void toggle_led(int led_num);

#ifdef __cplusplus
extern "C" {
#endif

void task_leds(void *params);

#ifdef __cplusplus
}
#endif
