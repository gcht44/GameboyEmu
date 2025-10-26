#include "../includes/ppu.h"
#include "../includes/cpu.h"
#include "../includes/video.h"
#include "../includes/lcd.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct ppu
{
    uint8_t ly;
    uint16_t dot;
    STAT_mode mode;
    uint8_t x;
    bool irq;
    uint8_t STAT;
    uint8_t lyc;
    uint8_t SCY;
    uint8_t SCX;
    uint8_t col;
    uint8_t tile_y;
    uint8_t WY;
    uint8_t WX;
    uint16_t window_line_counter;
} s_PPU;

static s_PPU ppu_struct = {0};
static s_LCD lcd = {0};
static uint8_t framebuffer[H*W];
static int fb_index;

static uint8_t vram[0x2000];
static uint8_t oam[0xA0];

static bool vram_acces = true;
static bool render_window;
static bool is_draw;

typedef struct oam_sprite_ctx 
{
    uint8_t y, x, tile_index, flags;
} OS_ctx;

static OS_ctx oam_scan[10];
static uint8_t oam_scan_size;
static bool oam_scan_done;

void STAT_check_ly()
{
    bool last_is_equal = (ppu_struct.STAT & 0x04) != 0;
    bool is_equal = ppu_struct.ly == ppu_struct.lyc;
    ppu_struct.STAT = (ppu_struct.STAT & ~0x04) | (is_equal ? 0x04 : 0);

    if (!last_is_equal && is_equal && (ppu_struct.STAT & 0x40))
        cpu_request_interrupt(IT_LCD_STAT);
}

void vram_write(uint16_t addr, uint8_t value)
{
    if (vram_acces)
    {
        addr -= 0x8000;
        vram[addr] = value;
    }
}

uint8_t vram_read(uint16_t addr)
{
    addr -= 0x8000;
    return vram[addr];
}

void STAT_write(uint8_t value) { ppu_struct.STAT = (value & 0xFC) | (ppu_struct.STAT & 0x07); }

uint8_t STAT_read()            { return ppu_struct.STAT; }

void lyc_write(uint8_t value)
{
    ppu_struct.lyc = value;
    STAT_check_ly();
}

uint8_t lyc_read()             { return ppu_struct.lyc; }

uint8_t read_ly()              { return ppu_struct.ly; }

void SCX_write(uint8_t value)  { ppu_struct.SCX = value; }
uint8_t SCX_read()             { return ppu_struct.SCX; }

void SCY_write(uint8_t value)  { ppu_struct.SCY = value; }
uint8_t SCY_read()             { return ppu_struct.SCY; }

void WY_write(uint8_t value)   { ppu_struct.WY = value; }
uint8_t WY_read()              { return ppu_struct.WY; }
void WX_write(uint8_t value)   { ppu_struct.WX = value; }
uint8_t WX_read()              { return ppu_struct.WX; }

void oam_write(uint16_t addr, uint8_t v) 
{  
    addr -= 0xFE00;
    oam[addr] = v;
}
uint8_t oam_read(uint16_t addr) { return oam[addr-0xFE00]; }

static void ppu_begin_line()
{
    ppu_struct.x = 0;
    ppu_struct.col = 0;
    lcd = get_lcd();
    oam_scan_done = false;
    render_window = false;
}

void STAT_change_mode(STAT_mode mode)
{
    ppu_struct.STAT = (ppu_struct.STAT & 0xFC) | (mode & 0x03);
    switch (mode)
    {
        case HBLANK:
            if ((ppu_struct.STAT & 0x08) != 0)
                cpu_request_interrupt(IT_LCD_STAT);
            break;
        case VBLANK:
            if ((ppu_struct.STAT & 0x10) != 0)
                cpu_request_interrupt(IT_LCD_STAT);
            break;
        case OAM_SCAN:
            if ((ppu_struct.STAT & 0x20) != 0)
                cpu_request_interrupt(IT_LCD_STAT);
        default:
            break;
    }
}

void set_stat_mode()
{
    if (ppu_struct.ly >= 144)
    {
        if (!ppu_struct.irq)
        {
            cpu_request_interrupt(IT_VBLANK);
            fb_index = 0;
            ppu_struct.irq = true;
            is_draw = false;
        }
        ppu_struct.mode = VBLANK;
        STAT_change_mode(ppu_struct.mode);
        vram_acces = true;
    }
    else if ((ppu_struct.dot <= 79) && (ppu_struct.ly <= 143))
    {
        ppu_struct.mode = OAM_SCAN;
        STAT_change_mode(ppu_struct.mode);
        vram_acces = true;
    }
    else if ((ppu_struct.dot >= 80) && (ppu_struct.x < 160))
    {
        ppu_struct.mode = DRAWING;
        STAT_change_mode(ppu_struct.mode);
        vram_acces = false;
    }
    else if ((ppu_struct.dot <= 455))
    {
        ppu_struct.mode = HBLANK;
        STAT_change_mode(ppu_struct.mode);
        vram_acces = true;
    }
}

void fill_oam_buff()
{
    uint8_t sprite_y;
    uint8_t sprite_x;
    uint8_t tile_index;
    uint8_t flags;
    int i_oam = 0;

    for (int i=0 ; i<4*40; i+=4)
    {
        if (i_oam >= 10)
        {
            break;
        }

        sprite_y = oam[i];
        sprite_x = oam[i+1];
        tile_index = oam[i+2];
        flags = oam[i+3];

        if (((sprite_y-16) <= ppu_struct.ly) && ((sprite_y-16 + lcd.obj_size_8x16) > ppu_struct.ly))
        {
            oam_scan[i_oam].y = sprite_y;
            oam_scan[i_oam].x = sprite_x;
            oam_scan[i_oam].tile_index = tile_index;
            oam_scan[i_oam].flags = flags;
            i_oam++;
        }
    }
    oam_scan_size = i_oam;
}

uint8_t render_sprite(OS_ctx oam_ctx, uint8_t pixel_x_pos)
{
    uint8_t tile_index;
    uint8_t pixel_y = ppu_struct.ly - (oam_ctx.y - 16);
    uint8_t pixel_x = pixel_x_pos - (oam_ctx.x - 8);
    
    if (pixel_x >= 8) return 0;

    bool flip_x = (oam_ctx.flags & 0x20) != 0;
    bool flip_y = (oam_ctx.flags & 0x40) != 0;
    
    if (flip_y)
    {
        if (lcd.obj_size_8x16 == 8)
            pixel_y = 7 - pixel_y;
        else
            pixel_y = 15 - pixel_y;
    }

    if (lcd.obj_size_8x16 == 8)
    {
        tile_index = oam_ctx.tile_index;
    }
    else
    {
        if (pixel_y < 8)
        {
            tile_index = oam_ctx.tile_index & 0xFE;
        }
        else
        {
            tile_index = (oam_ctx.tile_index & 0xFE) + 1;
            pixel_y -= 8;
        }
    }

    uint16_t tile_addr = 0x8000 + (tile_index * 16) + (pixel_y * 2);
    uint8_t lsb = vram[tile_addr - 0x8000];
    uint8_t msb = vram[tile_addr + 1 - 0x8000];

    if (flip_x)
        pixel_x = 7 - pixel_x;

    uint8_t bit_pos = 7 - pixel_x;
    uint8_t color_id = (((msb >> bit_pos) & 1) << 1) | ((lsb >> bit_pos) & 1);
    
    if (color_id == 0) return 0;

    uint8_t palette_flag = (oam_ctx.flags & 0x10) != 0;
    uint8_t *sprite_palette = get_obj_index_color(palette_flag);
    
    return sprite_palette[color_id];
}

uint8_t render_bg_pixel()
{
    if (!lcd.bg_enable) 
    {
        return lcd.index_color[0];
    }
    
    uint8_t scroll_y;
    uint8_t scroll_x;
    uint16_t tilemap_base;

    if (lcd.window_enable && (ppu_struct.ly >= ppu_struct.WY) && (ppu_struct.x >= ppu_struct.WX-7))
    {
        render_window = true;
        tilemap_base = lcd.window_tilemap_base;
        scroll_y = ppu_struct.window_line_counter;
        scroll_x = ppu_struct.x - (ppu_struct.WX-7);
    }
    else if (render_window)
    {
        tilemap_base = lcd.window_tilemap_base;
        scroll_y = ppu_struct.window_line_counter;
        scroll_x = ppu_struct.x - (ppu_struct.WX-7);
    }
    else
    {
        tilemap_base = lcd.bg_tilemap_base;
        scroll_y = (ppu_struct.ly + ppu_struct.SCY) & 0xFF;
        scroll_x = (ppu_struct.x + ppu_struct.SCX) & 0xFF;
    }
    
    uint8_t tile_y = scroll_y / 8;
    uint8_t tile_x = scroll_x / 8;
    uint8_t pixel_y = scroll_y % 8;
    uint8_t pixel_x = scroll_x % 8;
    
    uint16_t tilemap_addr = tilemap_base + (tile_y * 32) + tile_x;
    uint8_t tile_index = vram[tilemap_addr - 0x8000];
    
    uint16_t tile_addr;
    if (lcd.tile_data_base == 0x8000) 
        tile_addr = 0x8000 + (tile_index * 16) + (pixel_y * 2);
    else
        tile_addr = 0x9000 + (((int8_t)tile_index) * 16) + (pixel_y * 2);
    
    uint8_t lsb = vram[tile_addr - 0x8000];
    uint8_t msb = vram[tile_addr + 1 - 0x8000];

    uint8_t bit_pos = 7 - pixel_x;
    uint8_t color_id = (((msb >> bit_pos) & 1) << 1) | ((lsb >> bit_pos) & 1);
    
    return lcd.index_color[color_id];
}

void render_pixel()
{
    uint8_t bg_pixel = render_bg_pixel();
    uint8_t sprite_pixel = 0;
    uint8_t sprite_color_id = 0;
    bool sprite_bg_priority = false;
    int8_t highest_priority_sprite = -1;
    uint8_t lowest_x = 255;
    
    if (lcd.obj_enable) // Pour rendre un pixel d'un sprite
    {
        for (int i_oam = 0; i_oam < oam_scan_size; i_oam++) // trouver le sprite avec la plus petite coordonnée X
        {
            if ((ppu_struct.x >= oam_scan[i_oam].x - 8) && (ppu_struct.x < oam_scan[i_oam].x))
            {
                if (oam_scan[i_oam].x < lowest_x)
                {
                    lowest_x = oam_scan[i_oam].x;
                    highest_priority_sprite = i_oam;
                }
                else if (oam_scan[i_oam].x == lowest_x)
                {
                    continue;
                }
            }
        }
        
        if (highest_priority_sprite != -1)
        {
            int i_oam = highest_priority_sprite;
            uint8_t pixel_x = ppu_struct.x - (oam_scan[i_oam].x - 8);
            uint8_t pixel_y = ppu_struct.ly - (oam_scan[i_oam].y - 16);
            uint8_t tile_index;

            bool flip_x = (oam_scan[i_oam].flags & 0x20) != 0;
            bool flip_y = (oam_scan[i_oam].flags & 0x40) != 0;
            sprite_bg_priority = (oam_scan[i_oam].flags & 0x80) != 0;
            
            if (flip_y)
            {
                if (lcd.obj_size_8x16 == 8)
                    pixel_y = 7 - pixel_y;
                else
                    pixel_y = 15 - pixel_y;
            }
            
            if (flip_x)
                pixel_x = 7 - pixel_x;

            if (lcd.obj_size_8x16 == 8)
            {
                tile_index = oam_scan[i_oam].tile_index;
            }
            else
            {
                if (pixel_y < 8)
                {
                    tile_index = oam_scan[i_oam].tile_index & 0xFE;
                }
                else
                {
                    tile_index = (oam_scan[i_oam].tile_index & 0xFE) + 1;
                    pixel_y -= 8;
                }
            }
            
            uint16_t tile_addr = 0x8000 + (tile_index * 16) + (pixel_y * 2);
            uint8_t lsb = vram[tile_addr - 0x8000];
            uint8_t msb = vram[tile_addr + 1 - 0x8000];
            
            uint8_t bit_pos = 7 - pixel_x;
            sprite_color_id = (((msb >> bit_pos) & 1) << 1) | ((lsb >> bit_pos) & 1);
            
            if (sprite_color_id != 0)
            {
                uint8_t palette_flag = (oam_scan[i_oam].flags & 0x10) != 0;
                uint8_t *sprite_palette = get_obj_index_color(palette_flag);
                sprite_pixel = sprite_palette[sprite_color_id];
            }
        }
    }
    

    uint8_t final_pixel;
    
    if (sprite_pixel != 0)
    {
        uint8_t bg_color_id = bg_pixel;
        if (sprite_bg_priority && (bg_color_id != lcd.index_color[0]))
        {
            final_pixel = bg_pixel;
        }
        else
        {
            final_pixel = sprite_pixel;
        }
    }
    else
    {
        final_pixel = bg_pixel;
    }
    
    framebuffer[fb_index++] = final_pixel;
}

void ppu_tick()
{
    lcd = get_lcd();

    if (lcd.lcd_on)
    {
        set_stat_mode();
        
        if ((ppu_struct.mode == OAM_SCAN) && !oam_scan_done)
        {
            oam_scan_size = 0;
            fill_oam_buff();
            oam_scan_done = true;
        }
        else if (ppu_struct.mode == DRAWING)
        {
            if (ppu_struct.x < 160)
            {
                render_pixel();
                ppu_struct.x++;
            }
        }
        else if (ppu_struct.mode == VBLANK && !is_draw)
        {
            is_draw = true;
            draw_pixels(framebuffer);
        }
        
        // Horloge du PPU
        ppu_struct.dot += 1;
        if (ppu_struct.dot == 456)
        {
            if (render_window)
                ppu_struct.window_line_counter++;
            
            ppu_struct.dot = 0;
            ppu_struct.ly += 1;
            STAT_check_ly();

            // Reset de fin de ligne
            if (ppu_struct.ly == 154)
            {
                ppu_struct.ly = 0;
                ppu_struct.irq = false;
                ppu_struct.window_line_counter = 0;
            }
            ppu_begin_line();
        }
    }
}