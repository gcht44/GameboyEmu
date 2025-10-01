#include <stdio.h>

#include "../includes/video.h"

#define W 160
#define H 144
// #define SCALE 2

static SDL_Window *win = NULL;
static SDL_Renderer *ren = NULL;
static SDL_Texture *ptex = NULL;

static uint32_t fb[H*W];

/*void draw_test_checker(uint32_t* framebuffer)
{
    // 1) Remplir un damier visible (8x8 pixels)
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int v = ((x >> 3) ^ (y >> 3)) & 1;
            framebuffer[y*W + x] = v ? 0xFFFFFFFFu : 0xFF000000u; // blanc/noir opaques
        }
    }

    draw_pixels(framebuffer);
}*/

void convert_frame_buffer(const uint8_t *fb_src)
{
    uint32_t pixel_color;

    for (int i=0 ; i<H*W ; i++)
    {
        if ((fb_src[i] & 3) == 0)         // gris très clair
            pixel_color = 0xFFE0E0E0u;
        else if ((fb_src[i] & 3) == 1)         // gris clair
            pixel_color = 0xFFA8A8A8u; 
        else if ((fb_src[i] & 3) == 2)         // gris foncé
            pixel_color = 0xFF707070u;
        else if ((fb_src[i] & 3) == 3)         // noir/gris très sombre
            pixel_color = 0xFF202020u;
        else                             // Noir si jamais
            pixel_color = 0xFF000000u;
        fb[i] = pixel_color;
    }
}

int init_sdl()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        return 1;
    }

    win = SDL_CreateWindow("Intel8080 - SpaceInvader", W, H, 0);
    if (!win) {
        SDL_Log("CreateWindow: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    ren = SDL_CreateRenderer(win, NULL);
    if (!ren) {
        SDL_Log("CreateRenderer: %s", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderVSync(ren, 1);
    /*int running = 1;
    while (running) {
        // 1) événements
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = 0;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) running = 0;
        }
    }*/
    return 0;
}

void SDL_exit()
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}


// W et H sont les dimensions logiques de l’image, p.ex. 224x256
// framebuffer est un tableau W*H en ARGB8888 (uint32_t par pixel)
void draw_pixels(const uint8_t* framebuffer)
{
    // Créer la texture une fois si besoin
    if (!ptex) 
    {
        ptex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, W, H);
        if (!ptex) 
        {
            SDL_Log("CreateTexture: %s", SDL_GetError());
            return;
        }
    }

    convert_frame_buffer(framebuffer);

    void *pixels = NULL;
    int pitch = 0;
    SDL_LockTexture(ptex, NULL, &pixels, &pitch);

    // Copier ligne par ligne en respectant pitch
    uint8_t *dst = (uint8_t*)pixels;
    for (int y = 0; y < H; y++) 
    {
        memcpy(dst + y*pitch, &fb[y*W], W * sizeof(uint32_t));
    }
    SDL_UnlockTexture(ptex);

    SDL_RenderClear(ren);
    SDL_RenderTexture(ren, ptex, NULL, NULL);
    SDL_RenderPresent(ren);
}


void print_version_sdl3()
{
    printf("%d", SDL_GetVersion());
}