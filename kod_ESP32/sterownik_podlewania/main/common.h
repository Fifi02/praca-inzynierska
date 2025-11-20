#pragma once
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

extern int32_t global_x;
extern EventGroupHandle_t xISREventGroup;

#define BTN0_PUSHED    ( 1U << 0 )        //uwaga przyski w programie numerowane od 0
#define BTN1_PUSHED     ( 1U << 1 )       //uwaga przyski w programie numerowane od 0
#define BTN2_PUSHED     ( 1U << 2 )      //uwaga przyski w programie numerowane od 0
#define BTN3_PUSHED     ( 1U << 3 )      //uwaga przyski w programie numerowane od 0


#define SECTION_MAX_NUM 32
#define STATIONARY_SECTIONS 5
extern uint16_t section_num;

