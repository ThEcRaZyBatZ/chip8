#include "../headers/main.h"

void opcode_fetch(struct CPU* cpu){
    if(cpu->PC<=MEMORY_SIZE-2){
        cpu->opcode=cpu->memory[cpu->PC]<<8|cpu->memory[cpu->PC+1];
        
    }
    else{
        SDL_Log("pc out of bounds");
    }
}

void opcode_execute(struct CPU* cpu){
    //Nxyn
    uint8_t N=(cpu->opcode>>12)&0xF;
    uint8_t x=(cpu->opcode>>8)&0xF;
    uint8_t y=(cpu->opcode>>4)&0xF;
    uint8_t n=(cpu->opcode)&0xF;

    switch(N){
        case 0x0:
            //0___ family
            //CLS
            switch(cpu->opcode&0xFF){
                case(0xE0):
                    //00E0
                    //all pixels initialised
                    for(int i=0;i<64*32;i++){
                        cpu->video[i]=false;
                    }
                    cpu->PC+=2;
                    break;
                case(0xEE):
                    //00EE
                    //return from a subroutine
                    cpu->SP-=1;
                    cpu->PC=cpu->stack[cpu->SP];
                    break;

                default:
                    break;
            }
            break;
        
        case 0x1:
            //1nnn
            //1___ family
            cpu->PC=cpu->opcode & 0x0FFF;
            break;
        
        case 0x2:
            //2nnn
            //2___ family
            cpu->stack[cpu->SP]=cpu->PC+2;
            cpu->SP++;
            cpu->PC=cpu->opcode&0x0FFF;
            break;

        case 0x3:
            //3___ family
            //3xkk
            cpu->PC+=(((cpu->opcode)&0xFF)==cpu->V[x])?4:2;
            break;

        case 0x4:
            //4__ family
            //4xkk
            cpu->PC+=(((cpu->opcode)&0xFF)==cpu->V[x])?2:4;
            break;

        case 0x5:
            //5__family
            //5xy0
            cpu->PC+=(((cpu->opcode & 0xF)==0)&&(cpu->V[x]==cpu->V[y])) ? 4:2;
            break;

        case 0x6: 
            /*6___ family, 
            LD Vx,byte
            6xkk
            takes kk and puts it in Vx*/
            cpu->V[x]=cpu->opcode&0xFF;
            cpu->PC+=2;
            break;

        case 0x7:
            /*7___ family,
            ADD Vx,byte
            7xkk
            Vx <- Vx + kk
            */
           cpu->V[x]=cpu->V[x] + (cpu->opcode&0xFF);
           cpu->PC+=2;
           break;
        
        case 0x8:
            // 8xyn family ALU Family
           switch(n){
            case 0x0:
                // 8xy0
                // LD Vx, Vy
                // Vx<-Vy
                cpu->V[x]=cpu->V[y];
                cpu->PC+=2;
                break;

            case 0x1:
                // 8xy1
                // OR Vx,Vy
                // Vx <- Vx | Vy 
                cpu->V[x]=cpu->V[x]|cpu->V[y];
                cpu->PC+=2;
                break;

            case 0x2:
                // 8xy2
                // AND Vx,Vy
                // Vx <- Vx & Vy 
                cpu->V[x]=cpu->V[x]&cpu->V[y];
                cpu->PC+=2;
                break;

            case 0x3:
                // 8xy3
                // XOR Vx,Vy
                // Vx <- Vx ^ Vy 
                cpu->V[x]=cpu->V[x]^cpu->V[y];
                cpu->PC+=2;
                break;

            case 0x4:
                // 8xy4
                // ADD Vx,Vy
                // Vx <- Vx + Vy, carry VF =1
                uint16_t sum=cpu->V[x]+cpu->V[y];
                cpu->V[15]=(sum>255)?1:0;   // VF
                cpu->V[x]=sum&0xFF;             
                cpu->PC+=2;
                break;
            
            case 0x5:
                // 8xy5
                // SUB Vx,Vy
                // Vx <- Vx-Vy
                cpu->V[15]=(cpu->V[x]>=cpu->V[y])?1:0;
                cpu->V[x]=cpu->V[x]-cpu->V[y];
                cpu->PC+=2;
                break;

            case 0x6:
                // 8xy6
                // vf=lsb of v[x]
                // shift v[x] right by 1 
                cpu->V[15]=cpu->V[x]&0x1;
                cpu->V[x]=cpu->V[x]>>1;
                cpu->PC+=2;
                break;
            
            case 0x7:
                // 8xy7
                // SUBN Vy-Vx
                // Vx=Vy-Vx, VF 1 if no borrow else 0
                cpu->V[15]=(cpu->V[y]>=cpu->V[x])?1:0;
                cpu->V[x]=cpu->V[y]-cpu->V[x];
                cpu->PC+=2;
                break;
            
            case 0xE:
                //8xyE
                //Vf=MSB of V[x]
                //Vx<<1
                cpu->V[15]=(cpu->V[x]>>7)&(0x1);
                cpu->V[x]=cpu->V[x]<<1;
                cpu->PC+=2;
                break;

           default:
                SDL_Log("Unimplemented opcode of 8 family: %04X", cpu->opcode);
                break;
           }
           break;
        
        case 0x9:
           //9___ family
           //SNE Vx, Vy
           //ignore the next instruction if V[x] and V[y] are not equal
           cpu->PC+=cpu->V[x]!=cpu->V[y]? 4 : 2;
           break;
        case 0xA:
           //A___ family
           //Annn
           //I=nnn
           cpu->I=cpu->opcode&0x0FFF;
           cpu->PC+=2;
           break;

        case 0xB:
           //B___ family
           //Bnnn
           //Jump to V0 + nnn
           cpu->PC= (cpu->V[0])+(cpu->opcode&0xFFF);
           break;

        case 0xC:
           //C___ family
           //Cxkk
           //Vx=(randomByte) AND kk
           cpu->V[x]= (rand()%256) & (cpu->opcode&0xFF);
           cpu->PC+=2;
           break;
        
        case 0xD:
           //D___ family
           //Dxyn draw xyn
           //ors the pixels, offset defined by vx and vy, and n is the height of the sprite to be drawn
           //I stores the first memory location and we s
            cpu->V[15] = 0;
            uint16_t offset = (cpu->V[y] % 32) * 64 + (cpu->V[x] % 64);
           
            for (int i = 0; i < n; i++) {
                uint8_t curr_memory = cpu->memory[cpu->I + i];
                uint16_t row_offset = ((cpu->V[y] + i) % 32) * 64;

                for (int j = 0; j < 8; j++) {
                    uint8_t spriteBit = (curr_memory >> (7 - j)) & 1;
                    if (!spriteBit) continue;
                
                    uint16_t index = row_offset + ((cpu->V[x] + j) % 64);
                
                    if (cpu->video[index]) {
                        cpu->V[15] = 1;
                    }
                    cpu->video[index] ^= 1;
                }
            }
            cpu->PC += 2;
            break;

        case 0xE:
            //E family
            //E___
            switch(cpu->opcode&0xFF){
                case(0x9E):
                    //Ex9E
                    //skip next expression if keypad[v[x]] is pressed
                    cpu->PC+=(cpu->keypad[cpu->V[x]])?4:2;
                    break;
                case(0xA1):
                    //ExA1
                    //skip next expression if keypad[v[x]] is not pressed
                    cpu->PC+=(cpu->keypad[cpu->V[x]])?2:4;
                    break;
                default: break;
            }
            break;
        
        case 0xF:
        //F family
        //F___
            switch(cpu->opcode & 0xFF){
                case(0x07):
                    //Fx07
                    //V[x]=delay_timer
                    cpu->V[x]=cpu->delay_timer;
                    cpu->PC+=2;
                    break;
                
                case(0x15):
                    //Fx15
                    //delay_timer=V[x]
                    cpu->delay_timer=cpu->V[x];
                    cpu->PC+=2;
                    break;
                
                case(0x18):
                    //Fx18
                    //sound_timer=V[x]
                    cpu->sound_timer=cpu->V[x];
                    cpu->PC+=2;
                    break;
                
                case(0x1E):
                    //Fx1E
                    //I+=V[x]
                    cpu->I+=cpu->V[x];
                    cpu->PC+=2;
                    break;
                
                case(0x29):
                    //Fx29
                    //FONTSET_START is where all the hexa decimals are, 0,1,2,3,4,5,6,7,8,9 and so on
                    //V[x] will be the hexa decimal, 5 is the width
                    //we'll store it in I

                    cpu->I = FONTSET_START + cpu->V[x] * 5;
                    cpu->PC += 2;
                    break;
                
                case(0x33):
                    //Fx33
                    //Vx of BCD is stored in memory
                    cpu->memory[cpu->I]=cpu->V[x]/100;
                    cpu->memory[cpu->I+1]=cpu->V[x]/10 % 10;
                    cpu->memory[cpu->I+2]=cpu->V[x] % 10;
                    cpu->PC+=2;
                    break;
                
                case(0x55):
                    //Fx55
                    //all the registers are stored in memory (V0 to Vx)
                    for(int i=0;i<=x;i++){
                        cpu->memory[cpu->I+i]=cpu->V[i];
                    }
                    cpu->PC+=2;
                    break;
                
                case(0x65):
                    //Fx65
                    //get 0 to x from memory
                    for(int i=0;i<=x;i++){
                        cpu->V[i]=cpu->memory[cpu->I+i];
                    }
                    cpu->PC+=2;
                    break;
                
                case(0x0A):
                    //we wait for input
                    //fx0A
                    cpu->waiting_for_key=true;
                    cpu->waiting_register=x;

                
                    break;

                default:
                    SDL_Log("Unimplemented opcode belonging to F family: %04X", cpu->opcode);
                    break;
            }
            break;
        
        default:
            SDL_Log("Unimplemented opcode: %04X", cpu->opcode);
            break;
    }
}

void cpu_cycle(struct CPU* cpu){
    if(!cpu->waiting_for_key){
        opcode_fetch(cpu);
        opcode_execute(cpu);
    }
}