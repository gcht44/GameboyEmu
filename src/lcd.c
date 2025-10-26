#include "../includes/lcd.h"

static s_LCD lcd;

static inline void build_palette(uint8_t reg, uint8_t out[4]) {
    out[0] = (reg >> 0) & 0x3;
    out[1] = (reg >> 2) & 0x3;
    out[2] = (reg >> 4) & 0x3;
    out[3] = (reg >> 6) & 0x3;
}

void lcdc_write(uint8_t v)
{
    lcd.LCDC = v;
    lcd.lcd_on = (v & 0x80) != 0;
    lcd.bg_enable = (v & 0x01) != 0;
    lcd.obj_enable = (v & 0x02) != 0;
    lcd.obj_size_8x16 = ((v & 0x04) != 0) ? 16 : 8;
    lcd.bg_tilemap_base = (v & 0x08) ? 0x9C00 : 0x9800;
    lcd.tile_data_base  = (v & 0x10) ? 0x8000 : 0x8800;
    lcd.window_enable   = (v & 0x20) != 0;
    lcd.window_tilemap_base = (v & 0x40) ? 0x9C00 : 0x9800;
}

uint8_t* get_obj_index_color(int which) { return lcd.obj_index_color[(which ? 1 : 0)]; }

void update_index_color()
{
    lcd.index_color[0] = (lcd.BGP >> (0*2)) & 3;
    lcd.index_color[1] = (lcd.BGP >> (1*2)) & 3;
    lcd.index_color[2] = (lcd.BGP >> (2*2)) & 3;
    lcd.index_color[3] = (lcd.BGP >> (3*2)) & 3;
}

void update_obj_palettes()
{
    // Mise à jour de la palette OBP0
    build_palette(lcd.OBP0, lcd.obj_index_color[0]);
    
    // Mise à jour de la palette OBP1
    build_palette(lcd.OBP1, lcd.obj_index_color[1]);
}

void init_lcd()
{
    lcdc_write(0x91);
    lcd.BGP = 0xFC;
    lcd.OBP0 = 0xFF;
    lcd.OBP1 = 0xFF;
    update_index_color();
    update_obj_palettes();
}

uint8_t lcdc_read()
{
    return lcd.LCDC;
}

void bgp_write(uint8_t value)
{
    lcd.BGP = value;
    update_index_color();
}

uint8_t bgp_read()
{
    return lcd.BGP;
}

void obp0_write(uint8_t value)
{
    lcd.OBP0 = value;
    update_obj_palettes();
}

uint8_t obp0_read()
{
    return lcd.OBP0;
}

void obp1_write(uint8_t value)
{
    lcd.OBP1 = value;
    update_obj_palettes();
}

uint8_t obp1_read()
{
    return lcd.OBP1;
}

s_LCD get_lcd()
{
    return lcd;
}

uint8_t *get_index_color()
{
    return lcd.index_color;
}