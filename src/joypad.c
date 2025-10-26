#include "../includes/joypad.h"
#include "../includes/cpu.h"  // Pour cpu_request_interrupt
#include <stdio.h>

static uint8_t joypad_reg;
static uint8_t button_states;
static uint8_t old_button_states; // Pour détecter les changements

void init_joypad()
{
    joypad_reg = 0xFF;
    button_states = 0xFF;
    old_button_states = 0xFF;
}

void update_joypad_state()
{
    uint8_t result = (joypad_reg & 0x30) | 0xC0;
    
    bool select_dpad = !(joypad_reg & 0x10);
    bool select_buttons = !(joypad_reg & 0x20);
    
    if (select_dpad)
    {
        result |= (button_states >> 4) & 0x0F;
    }
    
    if (select_buttons)
    {
        result |= button_states & 0x0F;
    }
    
    if (!select_dpad && !select_buttons)
    {
        result |= 0x0F;
    }
    
    joypad_reg = result;
}

void joypad_write(uint8_t value)
{
    joypad_reg = (joypad_reg & 0x0F) | (value & 0x30);
    update_joypad_state();
}

uint8_t joypad_read()
{
    return joypad_reg;
}

void update_joypad(KEY_JOY key, uint8_t value)
{
    uint8_t old_states = button_states;
    uint8_t bit_value = (value == 1) ? 0 : 1;
    
    switch (key)
    {
        case K_RIGHT:
            if (bit_value)
                button_states |= (1 << 4);
            else
                button_states &= ~(1 << 4);
            break;
            
        case K_LEFT:
            if (bit_value)
                button_states |= (1 << 5);
            else
                button_states &= ~(1 << 5);
            break;
            
        case K_UP:
            if (bit_value)
                button_states |= (1 << 6);
            else
                button_states &= ~(1 << 6);
            break;
            
        case K_DOWN:
            if (bit_value)
                button_states |= (1 << 7);
            else
                button_states &= ~(1 << 7);
            break;
            
        case K_A:
            if (bit_value)
                button_states |= (1 << 0);
            else
                button_states &= ~(1 << 0);
            break;
            
        case K_B:
            if (bit_value)
                button_states |= (1 << 1);
            else
                button_states &= ~(1 << 1);
            break;
            
        case K_SELECT:
            if (bit_value)
                button_states |= (1 << 2);
            else
                button_states &= ~(1 << 2);
            break;
            
        case K_START:
            if (bit_value)
                button_states |= (1 << 3);
            else
                button_states &= ~(1 << 3);
            break;
            
        default:
            break;
    }
    
    uint8_t changed_bits = old_states & ~button_states;
    
    if (changed_bits != 0)
    {
        bool select_dpad = !(joypad_reg & 0x10);
        bool select_buttons = !(joypad_reg & 0x20);
        
        bool interrupt_needed = false;
        
        if (select_dpad && (changed_bits & 0xF0))
        {
            interrupt_needed = true;
        }
        
        if (select_buttons && (changed_bits & 0x0F))
        {
            interrupt_needed = true;
        }
        
        if (interrupt_needed)
        {
            cpu_request_interrupt(IT_JOYPAD);
        }
    }
    
    update_joypad_state();
    // printf("Joypad: %02X (buttons: %02X)\n", joypad_reg, button_states);
}