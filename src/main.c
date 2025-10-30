#include <stdio.h>
#include <stdlib.h>

#include "../includes/cart.h"
#include "../includes/cpu.h"
#include "../includes/bus.h"
#include "../includes/video.h"
#include "../includes/lcd.h"
#include "../includes/joypad.h"

void handle_key_event(SDL_Event e)
{
    if (e.type == SDL_EVENT_KEY_UP)
    {
        switch (e.key.key)
        {
        case SDLK_Z: update_joypad(K_UP, 0); break;
        case SDLK_Q: update_joypad(K_LEFT, 0); break;
        case SDLK_D: update_joypad(K_RIGHT, 0); break;
        case SDLK_S: update_joypad(K_DOWN, 0); break;
        case SDLK_A: update_joypad(K_A, 0); break;
        case SDLK_E: update_joypad(K_B, 0); break;
        case SDLK_W: update_joypad(K_START, 0); break;
        case SDLK_X: update_joypad(K_SELECT, 0); break;
        }
    }
    else if (e.type == SDL_EVENT_KEY_DOWN)
    {
        switch (e.key.key)
        {
        case SDLK_Z: update_joypad(K_UP, 1); break;
        case SDLK_Q: update_joypad(K_LEFT, 1); break;
        case SDLK_D: update_joypad(K_RIGHT, 1); break;
        case SDLK_S: update_joypad(K_DOWN, 1); break;
        case SDLK_A: update_joypad(K_A, 1); break;
        case SDLK_E: update_joypad(K_B, 1); break;
        case SDLK_W: update_joypad(K_START, 1); break;
        case SDLK_X: update_joypad(K_SELECT, 1); break;
        }
    }
}

int main()
{
    s_CPU cpu;
    bool play_emu = true;
    
    const uint32_t TARGET_FPS = 60;
    const uint32_t FRAME_TIME_MS = 1000 / TARGET_FPS; // ~16.67ms
    const uint32_t CYCLES_PER_FRAME = 17556; // M-cycles per frame (70224 T-cycles / 4)
    
    uint32_t last_frame_time = 0;
    uint32_t accumulated_cycles = 0;

    load_rom("rom/Legend of Zelda, The - Link's Awakening (France).gb");
    init_cpu(&cpu);  
    init_lcd();
    init_joypad();  
    init_sdl();

    while(play_emu)
    {
        uint32_t current_time = SDL_GetTicks();
        
        while (accumulated_cycles < CYCLES_PER_FRAME) 
        {
            // Check event every 256 t-cycles
            if ((accumulated_cycles & 0xFF) == 0) 
            {
                SDL_Event e;
                while (SDL_PollEvent(&e)) 
                {
                    if (e.type == SDL_EVENT_QUIT) 
                        play_emu = false;
                    if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) 
                        play_emu = false;

                    handle_key_event(e);
                }
                if (!play_emu) break;
            }
            
            uint8_t cycles = step_emu(&cpu);
            accumulated_cycles += cycles;
        }
        
        // Reset cycles for the next frame
        accumulated_cycles -= CYCLES_PER_FRAME;
        
        if (current_time - last_frame_time >= FRAME_TIME_MS) 
        {
            last_frame_time = current_time;
        } 
        else 
        {
            // Wait
            uint32_t sleep_time = FRAME_TIME_MS - (current_time - last_frame_time);
            if (sleep_time > 0 && sleep_time < FRAME_TIME_MS) 
            {
                SDL_Delay(sleep_time);
            }
        }
    }

    SDL_exit();
    return 0;
}