#include "task_buttons.h"

static const char *TAG = "BUTTONS";

/* global states (opcjonalne) */
volatile bool btn_state[NUM_BUTTONS] = {0};

/* kolejka na zdarzenia z ISR */
QueueHandle_t xButtonQueue = NULL;

/* EventGroup – jeśli nadal chcesz sygnalizować przyciski wyżej */
extern EventGroupHandle_t xISREventGroup;   // albo zadeklaruj tu i utwórz w main

/* piny przycisków */
const gpio_num_t btn_pins[NUM_BUTTONS] = {
    BTN0_PIN, BTN1_PIN, BTN2_PIN, BTN3_PIN
};

/* ISR z debouncingiem + kolejka */
static void IRAM_ATTR btn_isr_handler(void *arg)
{
    int idx = (int)arg;   // który przycisk (0..NUM_BUTTONS-1)
    static uint32_t last_tick[NUM_BUTTONS] = {0};

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // --- DEBOUNCE W ISR (na tickach RTOS) ---
    uint32_t now = xTaskGetTickCountFromISR();
    if ((now - last_tick[idx]) < pdMS_TO_TICKS(200)) { // 50 ms
        return;  // ignoruj odbicie
    }
    last_tick[idx] = now;

    // odczytaj aktualny stan
    bool pressed = (gpio_get_level(btn_pins[idx]) == 0); // przycisk do GND + pull-up

    btn_state[idx] = pressed ? BTN_PRESSED : BTN_RELEASED;

    // przygotuj strukturę do kolejki
    ButtonIsrData_t data = {
        .button_pin = btn_pins[idx],
        .pressed    = pressed
    };

    // wyślij do kolejki
    if (xButtonQueue != NULL) {
        xQueueSendFromISR(xButtonQueue, &data, &xHigherPriorityTaskWoken);
    }

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

/* inicjalizacja GPIO i przerwań – teraz dostaje uchwyt kolejki */
void buttons_init(QueueHandle_t queue)
{
    xButtonQueue = queue;

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,          // tylko zbocze opadające
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,        // przycisk do GND
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pin_bit_mask = 0
    };

    for (int i = 0; i < NUM_BUTTONS; ++i) {
        io_conf.pin_bit_mask |= (1ULL << btn_pins[i]);
    }

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // serwis ISR (raz w projekcie)
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    for (int i = 0; i < NUM_BUTTONS; ++i) {
        ESP_ERROR_CHECK(gpio_isr_handler_add(btn_pins[i], btn_isr_handler, (void*)i));
        btn_state[i] = (gpio_get_level(btn_pins[i]) == 0) ? BTN_PRESSED : BTN_RELEASED;
    }

    ESP_LOGI(TAG, "Buttons initialized (queue + debounce).");
}

/* mapowanie przycisku na event bits – możesz zostawić EventGroup jak chciałeś */
static void handle_event(ButtonIsrData_t data)
{
    if (!data.pressed) {
        // jeśli interesuje Cię tylko wciśnięcie, a nie puszczenie – wyjdź
        return;
    }

    if (data.button_pin == BTN0_PIN) {
        xEventGroupSetBits(xISREventGroup, BTN0_PUSHED);
        printf("Btn1\n");
    } else if (data.button_pin == BTN1_PIN) {
        xEventGroupSetBits(xISREventGroup, BTN1_PUSHED);
         printf("Btn2\n");
    } else if (data.button_pin == BTN2_PIN) {
        xEventGroupSetBits(xISREventGroup, BTN2_PUSHED);
         printf("Btn3\n");
    } else if (data.button_pin == BTN3_PIN) {
        xEventGroupSetBits(xISREventGroup, BTN3_PUSHED);
         printf("Btn4\n");
    }
}

/* task obsługujący kolejkę */
void task_buttons(void *params)
{
    ESP_LOGI(TAG, "task_button starting...");

    // utwórz kolejkę (8 zdarzeń w buforze)
    xButtonQueue = xQueueCreate(8, sizeof(ButtonIsrData_t));
    configASSERT(xButtonQueue != NULL);

    // inicjalizacja GPIO + ISR z przekazaniem kolejki
    buttons_init(xButtonQueue);

    ButtonIsrData_t data;

    while (1)
    {
        if (xQueueReceive(xButtonQueue, &data, portMAX_DELAY) == pdTRUE)
        {
            // tu już jesteś w tasku – możesz robić „ciężkie” rzeczy
            handle_event(data);
        }
    }
}
