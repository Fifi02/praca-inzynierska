#ifndef LCD_I2C_H
#define LCD_I2C_H
#define MENU_DEPH 3
#include "driver/i2c.h"
#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>



#ifdef __cplusplus
extern "C" {
#endif

// Pin definitions - adjust these to match your hardware
#define SDA_PIN 22
#define SCL_PIN 21

enum{
    MAIN_MENU,
    SECTION_MENU,
    SETTINGS,
};

// Function declarations
esp_err_t lcd_i2c_init(void);
void lcd_init(uint8_t addr, uint8_t cols, uint8_t rows);
void lcd_clear(void);
void lcd_home(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *str);
void lcd_print_at(uint8_t col, uint8_t row, const char *str);
void lcd_backlight(bool state);
void lcd_display(bool state);
void lcd_cursor(bool state);
void lcd_blink(bool state);
void task_lcd(__unused void *params);
void display_default_section_screen(int section_num);
void screen_display(int page, int menu_page, int section_num);
#ifdef __cplusplus
}
#endif

#endif // LCD_I2C_H