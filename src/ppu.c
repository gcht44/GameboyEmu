#include "../includes/ppu.h"
#include "../includes/cpu.h"
#include "../includes/video.h"

typedef struct ppu
{
    uint8_t index_vram;
    uint8_t ly;
    uint16_t dot;
    uint8_t fifo[8];
    uint8_t fifo_index;
    STAT_mode mode;
    uint8_t x;
    uint8_t col; // Max 19 because one line of 1 tile is 8pixels so 8 * 20=160 W=160
    bool fetch_fifo;
    uint8_t fifo_size;
    bool irq;
} s_PPU;

static s_PPU ppu_struct = {0};
static uint8_t framebuffer[H*W];
static uint8_t fb_index;

static uint8_t vram[0x2000];

void vram_write(uint16_t addr, uint8_t value)
{
    addr -= 0x8000;
    vram[addr] = value;
}

uint8_t vram_read(uint16_t addr)
{
    addr -= 0x8000;
    return vram[addr];
}

void fill_fifo()
{
    uint8_t tile_row = ppu_struct.ly / 8; // Get the tile
    uint8_t row = ppu_struct.ly % 8; // Get row of the tile
    uint16_t tilemap_addr = 0x9800 + tile_row*32 + ppu_struct.col; // Get the tile addr (comme la tile map fait 32x32)
    uint8_t tile_id = vram_read(tilemap_addr);
    uint16_t tile_addr = 0x8000 + tile_id*16 + row*2; // Get addr of the 2bytes for 8 pixel at the good row

    uint8_t lsb = vram_read(tile_addr++);
    uint8_t msb = vram_read(tile_addr);
    uint8_t pixel;

    ppu_struct.fifo_index = 0;
    for (int i=7 ; i>=0 ; i--)
    {
        pixel = (((msb >> i) & 1) << 1) | ((lsb >> i) & 1);
        ppu_struct.fifo[ppu_struct.fifo_index++] = pixel;
    }
    ppu_struct.fifo_size = ppu_struct.fifo_index;
    ppu_struct.fifo_index = 0;

/*for (int i = 0; i < 8; i++) {
    // Exemple périodique visible: 0,1,2,3,0,1,2,3
    static const uint8_t k[8] = {0,1,2,3,0,1,2,3};
    ppu_struct.fifo[i] = k[i];
}
ppu_struct.fifo_index = 0;
ppu_struct.fifo_size  = 8;*/

//tlma = tilemap_addr; // TEMP
//ti = tile_id;
//    ta = tile_addr; // TEMP
}

void set_stat_mode()
{
    if (ppu_struct.ly >= 144)
    {
        if (!ppu_struct.irq)
        {
            cpu_request_interrupt(VBLANK);
            fb_index = 0;
            ppu_struct.irq = true;
        }
        ppu_struct.mode = VBLANK;
    }
    else if ((ppu_struct.dot <= 79) && (ppu_struct.ly <= 143))
        ppu_struct.mode = OAM_SCAN;
    else if ((ppu_struct.dot >= 80) && (ppu_struct.x < 160)) // On doit sortir 160 pixels
    {
        ppu_struct.mode = DRAWING;
    }
    else if ((ppu_struct.dot <= 455))
    {
        ppu_struct.mode = HBLANK;
    }
}

void ppu_tick()
{
    set_stat_mode();
    if ((ppu_struct.mode == DRAWING))
    {
        if ((ppu_struct.fifo_index < ppu_struct.fifo_size) && ((ppu_struct.x < 160)))
        {
            framebuffer[fb_index++] = ppu_struct.fifo[ppu_struct.fifo_index++];

            ppu_struct.x++;
            ppu_struct.col = ppu_struct.x/8;
        }

        else if ((ppu_struct.fifo_index == ppu_struct.fifo_size) && (ppu_struct.col < 20))
            fill_fifo();
    }
    else if (ppu_struct.mode == VBLANK)
    {
        draw_pixels(framebuffer);
    }
    
    // Horloge du PPU
    ppu_struct.dot += 1;
    if (ppu_struct.dot == 456)
    {
        ppu_struct.dot = 0;
        ppu_struct.ly += 1;

        if (ppu_struct.x==160)
        {
            // Reset de fin de ligne
            ppu_struct.x = 0;
            ppu_struct.col = 0;
            ppu_struct.fifo_index = 0;
            ppu_struct.fifo_size = 0;
        }

        if (ppu_struct.ly == 0)
            ppu_struct.irq = false;
    }
}