#include "../headers/sdlfunc.h"
#include "../headers/main.h"
bool screen_init(struct display* d){
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");

    if(!SDL_Init(SDL_FLAGS)){
        SDL_Log("Display Init Failed");
        return false;

    }
    d->window=SDL_CreateWindow(TITLE,WIDTH,HEIGHT,SDL_FLAGS);
    if(d->window==NULL){
        SDL_Log("Error Creating window");
        return false;
    }

    d->renderer=SDL_CreateRenderer(d->window,NULL);
    if(d->renderer==NULL){
        SDL_Log("Error Creating renderer");
        return false;
    }
    d->is_running=true;
    return true;
}

int sdl_to_chip8(SDL_Keycode key) {
    switch(key){
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;
        case SDLK_Q: return 0x4;
        case SDLK_W: return 0x5;
        case SDLK_E: return 0x6;
        case SDLK_R: return 0xD;
        case SDLK_A: return 0x7;
        case SDLK_S: return 0x8;
        case SDLK_D: return 0x9;
        case SDLK_F: return 0xE;
        case SDLK_Z: return 0xA;
        case SDLK_X: return 0x0;
        case SDLK_C: return 0xB;
        case SDLK_Y: return 0xF;
        default: return -1;
    }
}

void screen_events(struct display* d,struct CPU* cpu){
    int chipkey=-1;
    while(SDL_PollEvent(&d->event)){
        switch(d->event.type){

            case SDL_EVENT_QUIT:
                d->is_running=false;
                break;

            case SDL_EVENT_KEY_UP:
                chipkey=sdl_to_chip8(d->event.key.key);
                if(chipkey!=-1){
                    cpu->keypad[chipkey]=0;
                }   
                break;
            
            case SDL_EVENT_KEY_DOWN:
                chipkey=sdl_to_chip8(d->event.key.key);
                if(chipkey!=-1 && !cpu->keypad[chipkey]){
                    cpu->keypad[chipkey]=1;
                    if(cpu->waiting_for_key){
                        cpu->V[cpu->waiting_register]=chipkey;

                        cpu->waiting_for_key=false;
                        cpu->waiting_register=-1;
                    }
                    cpu->keypad[chipkey]=1;
                }
                break;

            default:
                break;
        }
    }
}

void screen_draw(struct display* d,struct CPU* cpu){
    SDL_SetRenderDrawColor(d->renderer,0,0,0,255);
    SDL_RenderClear(d->renderer);

    SDL_SetRenderDrawColor(d->renderer,255,255,255,255);
    for(int i=0;i<32;i++){
        for(int j=0;j<64;j++){
            if(cpu->video[i*64+j]){
                SDL_FRect rect;
                rect.h=SCALE;
                rect.w=SCALE;
                rect.x=j*SCALE;
                rect.y=i*SCALE;
                SDL_RenderFillRect(d->renderer,&rect);
            }
        }
    }
    SDL_RenderPresent(d->renderer);
}

// void screen_run(struct display* d,struct CPU* cpu){ // MAIN RUN LOOP 
//     uint32_t last_timer_tick = SDL_GetTicks();
//     const int CPU_HZ = 700;
//     const int TIMER_HZ = 60;
//     while(d->is_running){
//         screen_events(d,cpu);
//         for(int i=0;i<CPU_HZ/TIMER_HZ;i++){
//             cpu_cycle(cpu);
//         }
//         if(SDL_GetTicks()-last_timer_tick >= 1000 / 60) { //60 Hz
//             if (cpu->delay_timer>0)cpu->delay_timer--;
//             if (cpu->sound_timer>0)cpu->sound_timer--;
//             last_timer_tick = SDL_GetTicks();
//         }
//         if(cpu->sound_timer>0){
//             printf("\a");
//         }
//         screen_draw(d,cpu);
//         SDL_Delay(1);
//     }
// }

void screen_run(struct display* d,struct CPU* cpu){
    const int CPU_HZ=700;  
    const int TIMER_HZ=60;
    const int CYCLES_PER_FRAME=CPU_HZ/60;
    const uint32_t FRAME_TIME=1000/60;
    
    uint32_t last_frame=SDL_GetTicks();
    uint32_t last_timer_tick=last_frame;
    int cycles_left = 0;
    
    while(d->is_running){
        uint32_t now=SDL_GetTicks();
        uint32_t delta_frame=now-last_frame;
        cycles_left+=delta_frame*(CPU_HZ / 1000.0f);
        
        while(cycles_left>0) {
            cpu_cycle(cpu);
            cycles_left--;
        }
        
        if(now-last_timer_tick>=1000/TIMER_HZ) {
            if(cpu->delay_timer>0)cpu->delay_timer--;
            if(cpu->sound_timer>0)cpu->sound_timer--;
            last_timer_tick=now;
        }
        
        screen_events(d,cpu);
        screen_draw(d,cpu);
        
        last_frame=now;
        uint32_t frame_time=SDL_GetTicks()-last_frame;
        if(frame_time<FRAME_TIME){
            SDL_Delay(FRAME_TIME-frame_time);
        }
    }
}


void screen_free(struct display* d){
    if(d->renderer){
        SDL_DestroyRenderer(d->renderer);
    }
    d->renderer=NULL;
    if(d->window){
        SDL_DestroyWindow(d->window);
    }
    d->window=NULL;
    SDL_Quit();
    SDL_Log("Everything Freed Succesfully");
}