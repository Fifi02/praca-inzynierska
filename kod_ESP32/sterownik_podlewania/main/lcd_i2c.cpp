#include "lcd_i2c.h"
#include "common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "rom/ets_sys.h"  // For ets_delay_us
#include "task_websocket.h"
#include <cstring>

static const char *TAG = "LCD";

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000

// LCD Commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET 0x20
#define LCD_SETDDRAMADDR 0x80

// Flags
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTDECREMENT 0x00
#define LCD_DISPLAYON 0x04
#define LCD_CURSOROFF 0x00
#define LCD_BLINKOFF 0x00
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_5x8DOTS 0x00

#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00
#define En 0x04
#define Rs 0x01

typedef struct {
    uint8_t addr;
    uint8_t cols;
    uint8_t rows;
    uint8_t backlight;
} lcd_t;

static lcd_t lcd;

esp_err_t lcd_write_byte(uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (lcd.addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C write failed: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

void lcd_write_nibble(uint8_t nibble) {
    uint8_t data = nibble | lcd.backlight;
    
    // Write data with En low
    lcd_write_byte(data);
    ets_delay_us(1);
    
    // Pulse En high
    lcd_write_byte(data | En);
    ets_delay_us(500);  // Keep En high for 500us - CRITICAL timing
    
    // Set En low
    lcd_write_byte(data & ~En);
    ets_delay_us(500);  // Wait before next operation
}

void lcd_send(uint8_t value, uint8_t mode) {
    uint8_t high = value & 0xF0;
    uint8_t low = (value << 4) & 0xF0;
    lcd_write_nibble(high | mode);
    lcd_write_nibble(low | mode);
}

void lcd_command(uint8_t cmd) {
    lcd_send(cmd, 0);
}

void lcd_data(uint8_t data) {
    lcd_send(data, Rs);
}

extern "C" esp_err_t lcd_i2c_init(void) {
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)SDA_PIN;
    conf.scl_io_num = (gpio_num_t)SCL_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = 0;
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C config failed: %s", esp_err_to_name(err));
        return err;
    }
    
    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(err));
    }
    
    return err;
}

extern "C" void lcd_init(uint8_t addr, uint8_t cols, uint8_t rows) {
    lcd.addr = addr;
    lcd.cols = cols;
    lcd.rows = rows;
    lcd.backlight = LCD_BACKLIGHT;
    
    ESP_LOGI(TAG, "Starting LCD initialization...");
    
    // Turn on backlight first
    lcd_write_byte(lcd.backlight);
    vTaskDelay(pdMS_TO_TICKS(100));  // Longer initial delay
    
    // Wait for LCD to power up (CRITICAL - some LCDs need this)
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Put LCD into 8-bit mode first (as per datasheet)
    lcd_write_nibble(0x30);
    vTaskDelay(pdMS_TO_TICKS(10));  // Must be >4.1ms
    
    lcd_write_nibble(0x30);
    vTaskDelay(pdMS_TO_TICKS(5));   // Must be >100us
    
    lcd_write_nibble(0x30);
    vTaskDelay(pdMS_TO_TICKS(5));
    
    // Now switch to 4-bit mode
    lcd_write_nibble(0x20);
    vTaskDelay(pdMS_TO_TICKS(5));
    
    // Function set: 4-bit mode, 2 lines, 5x8 dots
    lcd_command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
    vTaskDelay(pdMS_TO_TICKS(5));
    
    // Display OFF
    lcd_command(LCD_DISPLAYCONTROL);
    vTaskDelay(pdMS_TO_TICKS(5));
    
    // Clear display
    lcd_command(LCD_CLEARDISPLAY);
    vTaskDelay(pdMS_TO_TICKS(10));  // Clear needs >2ms
    
    // Entry mode set
    lcd_command(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
    vTaskDelay(pdMS_TO_TICKS(5));
    
    // Display ON
    lcd_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    vTaskDelay(pdMS_TO_TICKS(5));
    
    ESP_LOGI(TAG, "LCD initialized successfully");
}

extern "C" void lcd_clear(void) {
    lcd_command(LCD_CLEARDISPLAY);
    vTaskDelay(pdMS_TO_TICKS(5));  // Clear command needs longer delay
}

extern "C" void lcd_home(void) {
    lcd_command(LCD_RETURNHOME);
    vTaskDelay(pdMS_TO_TICKS(5));  // Home command needs longer delay
}

extern "C" void lcd_set_cursor(uint8_t col, uint8_t row) {
    // Row offsets for 20x4 LCD
    static const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    
    if (row >= lcd.rows) {
        row = lcd.rows - 1;
    }
    if (col >= lcd.cols) {
        col = lcd.cols - 1;
    }
    
    lcd_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
    ets_delay_us(100);  // Small delay after cursor set
}

extern "C" void lcd_print(const char *str) {
    if (str == NULL) return;
    
    while (*str) {
        lcd_data(*str++);
    }
}

extern "C" void lcd_print_at(uint8_t col, uint8_t row, const char *str) {
    lcd_set_cursor(col, row);
    lcd_print(str);
}

extern "C" void lcd_backlight(bool state) {
    lcd.backlight = state ? LCD_BACKLIGHT : LCD_NOBACKLIGHT;
    lcd_write_byte(lcd.backlight);
    ESP_LOGI(TAG, "Backlight %s", state ? "ON" : "OFF");
}

extern "C" void lcd_display(bool state) {
    if (state) {
        lcd_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    } else {
        lcd_command(LCD_DISPLAYCONTROL);
    }
}

extern "C" void lcd_cursor(bool state) {
    if (state) {
        lcd_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    } else {
        lcd_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    }
}

extern "C" void lcd_blink(bool state) {
    if (state) {
        lcd_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    } else {
        lcd_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    }
}


void task_lcd(__unused void *params)
{
    // Initialize LCD (20 columns x 4 rows)
    lcd_init(0x27, 20, 4);
    
    // Test backlight
    ESP_LOGI(TAG, "Testing backlight...");
    lcd_backlight(true);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Clear and display test text
    ESP_LOGI(TAG, "Displaying test text...");
    lcd_clear();
    lcd_print_at(0, 0, "ESP32 + LCD 20x4");
    lcd_print_at(0, 1, "I2C Address: 0x27");
    lcd_print_at(0, 2, "Line 3: Hello!");
    lcd_print_at(0, 3, "Line 4: World!");
    
    ESP_LOGI(TAG, "Text displayed, waiting 5 seconds...");
}



void display_default_section_screen(int section_num)
{
    lcd_clear();
    lcd_print_at(0, 0, "Sekcja numer: ");
    lcd_print_at(0, 1, "Planowany czas włączenia:");
    lcd_print_at(0, 2, ".., minut");
    lcd_print_at(0, 3, "1.<- 2.-> 3.Enter");
}

void display_default_section_menu(int section_num)
{
    lcd_clear();
    lcd_print_at(0, 0, "Sekcja numer: ");
    lcd_print_at(0, 1, "Wybierz program");
    lcd_print_at(0, 2, "Coś tu będzie");
    lcd_print_at(0, 3, "1.^2. 3.Enter4.Wróc");
}





void screen_display(int menu_page, int counter)
{
    switch (menu_page)
    {
        case MAIN_MENU:
        if(counter < section_num)
            display_default_section_screen(section_num);
        break;
        case SECTION_MENU:
            display_default_section_menu(counter);
        break;
    }

} 

int screen_limits = 10;
int counter=0;



void task_menu()
{
    uint8_t menu_page=0;
    while (true) 
    { 
        EventBits_t evBits = xEventGroupWaitBits( xISREventGroup, BTN0_PUSHED | BTN1_PUSHED | BTN2_PUSHED | BTN3_PUSHED, pdTRUE,pdFALSE,portMAX_DELAY);
        if (evBits & BTN0_PUSHED)
        {   
            counter--;    
            if(counter < 0)
            {
                counter =screen_limits;     
            }   
            screen_display(counter,menu_page);
        }
        if (evBits & BTN1_PUSHED)
        {   
            counter++;
            if(counter > screen_limits )
            {
                counter = 0; 
            }
        }
        if (evBits & BTN2_PUSHED)
        {   
            menu_page++;
            if(menu_page > MENU_DEPH)
            {
                menu_page=0;
            }
        }
        if (evBits & BTN3_PUSHED)
        {   
            menu_page--;
            if(menu_page  == 0)
            {
                menu_page=MENU_DEPH-1;
            }
        }
    }
}