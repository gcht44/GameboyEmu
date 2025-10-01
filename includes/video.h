#ifndef VIDEO__H
#define VIDEO__H

#include <../includes/SDL3/SDL.h>

#define W 160
#define H 144

void print_version_sdl3();
void draw_pixels(const uint8_t* framebuffer);
int init_sdl();
void SDL_exit();

#endif