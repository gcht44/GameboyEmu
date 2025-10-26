#ifndef JOYPAD__H
#define JOYPAD__H

#include <stdint.h>
#include <stdbool.h>

typedef enum KEY_JOY
{
    K_A,
    K_RIGHT,
    K_B,
    K_LEFT,
    K_SELECT,
    K_UP,
    K_DOWN,
    K_START,
}KEY_JOY;

void joypad_write(uint8_t value);
uint8_t joypad_read();
void update_joypad(KEY_JOY key, uint8_t value);
void init_joypad();

#endif