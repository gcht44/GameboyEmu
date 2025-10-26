#include <stdio.h>

#include "../includes/video.h"
#include "../includes/lcd.h"

#define W 160
#define H 144

static SDL_Window *win = NULL;
static SDL_Renderer *ren = NULL;
static SDL_Texture *ptex = NULL;

static uint32_t fb[H*W];

void convert_frame_buffer(const uint8_t *fb_src)
{
    uint32_t pixel_color;
    for (int i=0 ; i<H*W ; i++)
    {
        switch (fb_src[i])
        {
        case 0:
            pixel_color = 0xFFFFFFFF;
            break;
        case 1:
            pixel_color = 0xFFAAAAAA;
            break;
        case 2:
            pixel_color = 0xFF555555;
            break;
        case 3:
            pixel_color = 0xFF000000;
            break;
        default:
            pixel_color = 0xFFFF00FF; // Magenta pour debug
            break;
        }
        fb[i] = pixel_color;
    }
}

int init_sdl()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    win = SDL_CreateWindow("GameBoy Emulator", W, H, 0);
    if (!win) {
        SDL_Log("CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    ren = SDL_CreateRenderer(win, NULL);
    if (!ren) {
        SDL_Log("CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    
    // Activer VSync pour limiter naturellement à 60 FPS
    if (!SDL_SetRenderVSync(ren, 1)) {
        SDL_Log("Warning: VSync failed: %s", SDL_GetError());
    }
    
    return 0;
}

void SDL_exit()
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

void draw_pixels(const uint8_t* framebuffer)
{
    uint32_t last_render_time = 0;
    uint32_t TARGET_FPS = 60;
    uint32_t FRAME_TIME_MS = 1000 / TARGET_FPS; // ~16.67
    // Limiter le rendu à 60 FPS
    uint32_t current_time = SDL_GetTicks();
    if (current_time - last_render_time < FRAME_TIME_MS) {
        return; // Skip ce rendu si trop tôt
    }
    last_render_time = current_time;

    // Créer la texture une fois si besoin
    if (!ptex) {
        ptex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, 
                               SDL_TEXTUREACCESS_STREAMING, W, H);
        if (!ptex) {
            SDL_Log("Erreur: Impossible de créer la texture SDL: %s", SDL_GetError());
            return;
        }
    }

    // Vérifier que le framebuffer n'est pas NULL
    if (!framebuffer) {
        SDL_Log("Framebuffer is NULL!");
        return;
    }

    // Utiliser une palette précalculée pour une conversion plus rapide
    static const uint32_t palette[4] = {
        0xFFFFFFFF,  // Blanc
        0xFFAAAAAA,  // Gris clair
        0xFF555555,  // Gris foncé
        0xFF000000   // Noir
    };
    
    // Convertir le framebuffer en une seule passe
    for (int i = 0; i < H*W; i++) 
        fb[i] = palette[framebuffer[i] & 3];

    // Mise à jour de la texture et rendu
    if (!SDL_UpdateTexture(ptex, NULL, fb, W * sizeof(uint32_t))) {
        SDL_Log("Erreur SDL_UpdateTexture: %s", SDL_GetError());
        return;
    }

    if (!SDL_RenderClear(ren)) {
        SDL_Log("Erreur SDL_RenderClear: %s", SDL_GetError());
        return;
    }

    if (!SDL_RenderTexture(ren, ptex, NULL, NULL)) {
        SDL_Log("Erreur SDL_RenderTexture: %s", SDL_GetError());
        return;
    }

    if (!SDL_RenderPresent(ren)) {
        SDL_Log("Erreur SDL_RenderPresent: %s", SDL_GetError());
        return;
    }
}

void print_version_sdl3()
{
    printf("%d", SDL_GetVersion());
}