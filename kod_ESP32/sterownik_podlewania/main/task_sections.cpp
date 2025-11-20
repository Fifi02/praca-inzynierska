#include "task_sections.h"
#include "task_websocket.h"

static const char *TAG = "SECTIONS";


void sections_init(EventGroupHandle_t ev_group)
{
    for(int i=0;i <STATIONARY_SECTIONS;i++)
    {
        sections[i].section_pin_num = stationary_section_pin_num[i];
        sections[i].is_section_stationary=0;
        sections[i].is_section_stationary=0;
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << sections[i].section_pin_num),
            .mode = GPIO_MODE_OUTPUT,              // ustaw jako wyjście
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE         // brak przerwań dla wyjścia
        };
        gpio_config(&io_conf);
        ESP_ERROR_CHECK(gpio_config(&io_conf));
    }   
}





void update_online_section_state(int section_num, bool state)
{
    char msg[64];
    snprintf(msg, sizeof(msg),
         "Sekcja: %d, stan: %s",
         section_num,
         state ? "ON" : "OFF");

    esp_websocket_client_send_text(client, msg, strlen(msg), portMAX_DELAY);
}

void section_on(section_data section)
{
    if(section.is_section_stationary)
    {
        gpio_set_level(stationary_section_pin_num[section_num],1);
        section.state=1;
    }
    else
    {
        //wyśli do modułu zaworu
    }
    update_online_section_state(section_num,1);
}

// void section_off(int section_num)
// {
//     if(section.is_section_stationary)
//     {
//         gpio_set_level(section_pin[section_num],0);
//         section.state=0;
//     }
//     else
//     {
//         //wyśli do modułu zaworu
//     }
//     update_online_section_state(section_num);
// }


// void cauculate_next_turn_on(section_data section)
// {
//     if(section.is_section_stationary == 1)
//     {

//     } 
//     else
//     {

//     }
// }

void task_sections()
{
    ESP_LOGI(TAG, "task sections starting");
    while (true) 
    { 
        EventBits_t evBits = xEventGroupWaitBits( xISREventGroup, SECTION_ON | SECTION_OFF, pdTRUE,pdFALSE,portMAX_DELAY);
        if (evBits & SECTION_ON)
        {   
            printf("Section ON"); 
            for(int i=0;i < section_num;i++)
            {
                if(section_turn_on_flag[section_num]==1)
                {
                    section_on(sections[section_num]);
                }
            }
        }
        if (evBits & SECTION_OFF)
        {   
            printf("Section OFF"); 
        }
       
    }
}