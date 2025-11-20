#pragma once
#include "common.h"


#define SECTION_5_PIN GPIO_NUM_32

#define SECTION_ON    ( 1U << 0 )
#define SECTION_OFF    ( 1U << 1 )


bool section_turn_on_flag[SECTION_MAX_NUM ]={0};
bool section_turn_off_flag[SECTION_MAX_NUM ]={0};
struct section_data
{
    gpio_num_t  section_pin_num;
    bool is_section_stationary;
    bool state;
    //bool humidity
    //bool light
};

void sections_init();
//void cauculate_next_turn_on(section_data section);
//void section_off(int section_num);
void section_on(section_data section);

section_data sections[32];
gpio_num_t stationary_section_pin_num[STATIONARY_SECTIONS] = {SECTION_5_PIN};

