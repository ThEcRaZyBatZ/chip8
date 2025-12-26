#ifndef MAIN_H
#define MAIN_H

#include "sdlfunc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define ROM_SIZE 100000
#define MEMORY_SIZE 4096
#define FONTSET_START 0x50

struct CPU{

    uint8_t memory[MEMORY_SIZE];

    uint8_t V[16]; //V0 to VF with VF being the flag register (registers)
    uint16_t PC; //Program Counter
    uint16_t I; //12 bit, Index Register

    uint16_t stack[16];
    uint8_t SP;

    uint8_t delay_timer;
    uint8_t sound_timer;
    
    uint8_t keypad[16];
    bool video[64*32]; 

    uint16_t opcode;

    bool waiting_for_key;
    uint8_t waiting_register; // x Fx0A

    
};

bool screen_init(struct display* d);
void screen_free(struct display* d);
void screen_run(struct display* d,struct CPU* cpu);
void cpu_cycle(struct CPU* cpu);

#endif

