#ifndef SDLFUNC_H
#define SDLFUNC_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#define SCALE 14
#define HEIGHT 450
#define WIDTH 900
#define SDL_FLAGS SDL_INIT_VIDEO
#define TITLE "CHIP 8 Emulator"

#define PIXEL(x, y)[(y) * 64 + (x)]


struct display{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;
    bool is_running;
};


#endif