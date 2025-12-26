#include "../headers/main.h"

// to compile gcc main.c ./cpu_fun.c ./sdl_func.c -o ../output/chip_8 $(pkg-config --cflags --libs sdl3 sdl3-image) -lm
//make sure you have SDL3 ready


int main(){
    srand(time(NULL));
    struct display screen={0};
    struct CPU cpu={0};
    cpu.PC=0x200;
    FILE* f=fopen("../ROMs/tetris.ch8","rb");
    if(f==NULL){
        SDL_Log("ROM read Failed");
        screen_free(&screen);
        return -1;
    }
    fread(cpu.memory + 0x200, 1, ROM_SIZE, f);
    fclose(f);
    if(!screen_init(&screen)){
        screen_free(&screen);
        return -1;
    }
    screen_run(&screen,&cpu);
    screen_free(&screen);

}