#include <stdio.h>
#include <stdlib.h> // Ajout de stdlib.h pour malloc et free

#include "../includes/cart.h"
#include "../includes/cpu.h"
#include "../includes/bus.h"
#include "../includes/video.h"
#include "../includes/lcd.h"
#include "../includes/joypad.h"

int main()
{
    s_CPU cpu;
    bool play_emu = true;
    // unsigned lines = 0;

    // Ouvrir le fichier en mode binaire pour éviter les problèmes de traitement de texte
    /*FILE *fp = fopen("log/log.txt", "wb");
    if (!fp) {
        printf("Erreur: Impossible d'ouvrir le fichier de log\n");
        return -1;
    }*/

    /*char *buffer = malloc(32768); // 32KB buffer
    if (buffer) {
        setvbuf(fp, buffer, _IOFBF, 32768);
    }*/

    const uint32_t TARGET_FPS = 60;
    const uint32_t FRAME_TIME_MS = 1000 / TARGET_FPS; // ~16.67ms par frame
    uint32_t last_frame_time = 0;

    load_rom("rom/Tetris (World) (Rev 1).gb");
    // load_rom("rom/test_roms/instr_timing.gb");
    init_cpu(&cpu);  
    init_lcd();
    init_joypad();  
    init_sdl();

    while(play_emu)
    {
        // Écriture dans le fichier log
        /*fprintf(fp, "%04X:  A:%02x H:%02x L:%02x PCMEM:%02X,%02X,%02X,%02X IF:%02X IE:%02X IME:%d BGP:%02X LCD:%02X STAT:%02X LY:%02X LYC:%02X SCX:%02X SCY:%02X\n",
            cpu.pc, cpu.a, cpu.h, cpu.l,
            bus_read(cpu.pc, false), bus_read(cpu.pc+1, false), 
            bus_read(cpu.pc+2, false), bus_read(cpu.pc+3, false), 
            bus_read(0xFF0F, false), bus_read(0xFFFF, false), 
            cpu.ime, bus_read(0xFF47, false), bus_read(0xFF40, false), 
            bus_read(0xFF41, false), bus_read(0xFF44, false), bus_read(0xFF45, false), bus_read(0xFF43, false), bus_read(0xFF42, false));


        if ((++lines & 0x3FF) == 0) {
            fflush(fp);
        }*/
        uint32_t current_time = SDL_GetTicks();
                
        // Limiter les événements et le rendu à 60 FPS
        if (current_time - last_frame_time >= FRAME_TIME_MS)
        {
            SDL_Event e;
            while (SDL_PollEvent(&e)) 
            {
                if (e.type == SDL_EVENT_QUIT) play_emu = 0;
                if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) play_emu = 0;

                if (e.type == SDL_EVENT_KEY_UP)
                {
                    switch (e.key.key)
                    {
                    case SDLK_Z:
                        update_joypad(K_UP, 0);
                        break;
                    case SDLK_Q:
                        update_joypad(K_LEFT, 0);
                        break;
                    case SDLK_D:
                        update_joypad(K_RIGHT, 0);
                        break;
                    case SDLK_S:
                        update_joypad(K_DOWN, 0);
                        break;

                    case SDLK_A:
                        update_joypad(K_A, 0);
                        break;
                    case SDLK_E:
                        update_joypad(K_B, 0);
                        break;
                    case SDLK_W:
                        update_joypad(K_START, 0);
                        break;
                    case SDLK_X:
                        update_joypad(K_SELECT, 0);
                        break;
                    default:
                        break;
                    }
                }
                if (e.type == SDL_EVENT_KEY_DOWN)
                {
                    switch (e.key.key)
                    {
                    case SDLK_Z:
                        update_joypad(K_UP, 1);
                        break;
                    case SDLK_Q:
                        update_joypad(K_LEFT, 1);
                        break;
                    case SDLK_D:
                        update_joypad(K_RIGHT, 1);
                        break;
                    case SDLK_S:
                        update_joypad(K_DOWN, 1);
                        break;

                    case SDLK_A:
                        update_joypad(K_A, 1);
                        break;
                    case SDLK_E:
                        update_joypad(K_B, 1);
                        break;
                    case SDLK_W:
                        update_joypad(K_START, 1);
                        break;
                    case SDLK_X:
                        update_joypad(K_SELECT, 1);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        step_emu(&cpu);
    }

    // Nettoyer
    // fflush(fp);
    // fclose(fp);
    // free(buffer);
    SDL_exit();
    return 0;
}