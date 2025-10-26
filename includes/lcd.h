#ifndef LCD__H
#define LCD__H

#include <stdint.h>
#include <stdbool.h>

typedef struct LCD
{
    uint8_t LCDC;
    uint8_t BGP;
    bool lcd_on;
    bool bg_enable;
    uint16_t bg_tilemap_base;
    uint16_t tile_data_base;
    uint8_t index_color[4];

    bool window_enable;
    uint16_t window_tilemap_base;

    bool obj_enable;
    uint8_t obj_size_8x16;
    uint8_t OBP0, OBP1;
    uint8_t obj_index_color[2][4];
} s_LCD;

void init_lcd();
void lcdc_state();
void lcdc_write(uint8_t value);
s_LCD get_lcd();
uint8_t* get_index_color();
void bgp_write(uint8_t value);
uint8_t lcdc_read();
uint8_t bgp_read();
void obp0_write(uint8_t v);
void obp1_write(uint8_t v);
uint8_t obp0_read();
uint8_t obp1_read();
uint8_t* get_obj_index_color(int which);

#endif