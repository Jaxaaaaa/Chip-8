#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include <math.h>
#include <stdint.h>

typedef struct {
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint16_t stack[16];
    uint8_t stack_pointer;
    uint8_t delay_timer;
    uint8_t sound_timer;
    bool display[64][32];
    bool keypad[16];
} Chip8;

int main()
{

}
