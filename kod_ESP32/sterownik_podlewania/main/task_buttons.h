#pragma once
#include "common.h"

#define NUM_BUTTONS 4

#define BTN0_PIN    19  //uwaga przyski w programie numerowane od 0
#define BTN1_PIN    18  //uwaga przyski w programie numerowane od 0
#define BTN2_PIN    5   //uwaga przyski w programie numerowane od 0
#define BTN3_PIN    17  //uwaga przyski w programie numerowane od 0


#define BTN_PRESSED  1
#define BTN_RELEASED 0

extern const gpio_num_t btn_pins[NUM_BUTTONS];
extern const uint32_t btn_event_bits[NUM_BUTTONS];

typedef struct
{
    gpio_num_t button_pin;
    bool pressed;        // stan po filtracji (true = wciśnięty)
} ButtonIsrData_t;

extern QueueHandle_t xButtonQueue;

void buttons_init(QueueHandle_t queue);
#ifdef __cplusplus
extern "C" {
#endif

void task_buttons(void *params);



#ifdef __cplusplus
}
#endif

