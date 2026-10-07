    #include <stdio.h>
    #include <stdlib.h>
    #include "raylib.h"
    #include <math.h>
    #include <stdint.h>
    #define SCALE 10
    uint8_t font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
    const int keymap[16] ={
        KEY_X, //0 du chip 8
        KEY_ONE,//1
        KEY_TWO,//2
        KEY_THREE,//3
        KEY_Q,//4
        KEY_W, //5
        KEY_E, //6
        KEY_A, //7
        KEY_S, //8
        KEY_D, //9
        KEY_Z, //A
        KEY_C, //B
        KEY_FOUR, //C
        KEY_R, //D
        KEY_F, //E
        KEY_V, //F
        };
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
    Chip8 chip8;
    bool screen_drawn = false;
    int waiting_key = -1;
    void load_rom(const char *filename){
        FILE *file = fopen(filename, "rb");
        if (file == NULL){
            printf("Opening error\n");
            exit(1);
        }
        else{
            fread(&chip8.memory[0x200], 1, 3584, file);
            fclose(file);
        }
    }
    void draw_display(){
        for(int x = 0; x < 64 ; x++){
            for(int y = 0; y < 32; y++){
                if(chip8.display[x][y]){
                    DrawRectangle(x*SCALE, y*SCALE, SCALE, SCALE,  GREEN);
                }
            }
        }
    }

    void update_keypad(){
        for (int i = 0; i<16; i++){
            chip8.keypad[i] = IsKeyDown(keymap[i]);
        }
    }

    void emulate_cycle(){
            uint16_t instruction = (chip8.memory[chip8.pc] << 8) | chip8.memory[chip8.pc+1];
            //printf("%04X\n", instruction);
            chip8.pc += 2;
            uint8_t first_number = (instruction & 0xF000) >> 12;
            uint8_t X = (instruction & 0x0F00) >> 8;
            uint8_t Y = (instruction & 0x00F0) >> 4;
            uint8_t N = (instruction & 0x000f);
            uint8_t NN = (instruction & 0x00ff);
            uint16_t NNN= (instruction & 0x0fff);

            switch(first_number){
                case 0x0:
                    if(instruction == 0x00E0){
                        for(int x = 0; x<64; x++){
                            for(int y = 0; y < 32; y++){
                                chip8.display[x][y] = false;
                            }
                        }
                    }
                    else if(instruction == 0x00EE){
                        chip8.stack_pointer--;
                        chip8.pc = chip8.stack[chip8.stack_pointer];
                    }
                    break;
                case 0x1:
                    chip8.pc = NNN;
                    break;
                case 0x2:
                    chip8.stack[chip8.stack_pointer] = chip8.pc;
                    chip8.stack_pointer++;
                    chip8.pc = NNN;
                    break;
                case 0x3:
                    if(chip8.V[X] == NN){
                        chip8.pc += 2;
                    }
                    break;
                case 0x4:
                    if(chip8.V[X] != NN){
                        chip8.pc += 2;
                    }
                    break;
                case 0x5:
                    if(chip8.V[X] == chip8.V[Y]){
                        chip8.pc += 2;
                    }
                    break;
                case 0x6:
                    chip8.V[X] = NN;
                    break;
                case 0x7:
                    chip8.V[X] += NN;
                    break;
                case 0x8:
                    switch(N){
                    case 0x0:
                        chip8.V[X] = chip8.V[Y];
                        break;
                    case 0x1:
                        chip8.V[X] |= chip8.V[Y];
                        chip8.V[0xF] = 0;
                        break;
                    case 0x2:
                        chip8.V[X] &= chip8.V[Y];
                        chip8.V[0xF] = 0;
                        break;
                    case 0x3:
                        chip8.V[X] ^= chip8.V[Y];
                        chip8.V[0xF] = 0;
                        break;
                    case 0x4: {
                        uint16_t sum = chip8.V[X] + chip8.V[Y];
                        chip8.V[X] = sum;
                        if(sum > 255){
                            chip8.V[0xF] = 1;
                        }
                        else{
                            chip8.V[0xF] = 0;
                        }
                        break;
                    }
                    case 0x5:{
                        uint8_t flag = 0;
                        if(chip8.V[X] >= chip8.V[Y]){
                            flag = 1;
                        }
                        chip8.V[X] -= chip8.V[Y];
                        chip8.V[0xF] = flag;
                        break;
                    }
                    case 0x6:{
                        int flag = chip8.V[Y] & 0x01;
                        chip8.V[X] = chip8.V[Y] >> 1;
                        chip8.V[0xF] = flag;
                        break;
                    }
                    case 0x7:{
                        uint8_t flag = 0;
                        if(chip8.V[Y] >= chip8.V[X]){
                            flag = 1;
                        }
                        chip8.V[X] = chip8.V[Y] - chip8.V[X];
                        chip8.V[0xF] = flag;
                        break;
                    }
                    case 0xE:{
                        int flag = (chip8.V[Y] & 0x80) >> 7;
                        chip8.V[X] = chip8.V[Y] << 1;
                        chip8.V[0xF] = flag;
                        break;
                    }
                    }
                    break;
                case 0x9:
                    if(chip8.V[X] != chip8.V[Y]){
                        chip8.pc += 2;
                    }
                    break;
                case 0xA:
                    chip8.I = NNN;
                    break;
                case 0xB:
                    chip8.pc = NNN + chip8.V[0];
                    break;
                case 0xC:
                    chip8.V[X] = (GetRandomValue(0, 255) & NN);
                    break;
                case 0xD:{
                    int x = chip8.V[X] % 64;
                    int y = chip8.V[Y] % 32;
                    chip8.V[0xF] = 0;
                    for (int r = 0; r < N; r++){
                        if(y + r >= 32){
                            break;
                        }
                        int sprite_row = chip8.memory[chip8.I + r];
                        for (int c = 0; c < 8; c++){
                            if(x+c >= 64){
                                break;
                            }
                            if((sprite_row &(0x80 >> c)) != 0){
                                if(chip8.display[x+c][y+r] == 1){
                                chip8.V[0xF] = 1;
                                }
                                chip8.display[x+c][y+r] = !chip8.display[x+c][y+r];
                            }
                        }
                    }
                    screen_drawn = true;
                }
                    break;
                case 0xE:
                    switch(NN){
                        case 0x9E:
                            if(chip8.keypad[chip8.V[X]]){
                                chip8.pc +=2;
                            }
                            break;
                        case 0xA1:
                            if(!chip8.keypad[chip8.V[X]]){
                                chip8.pc +=2;
                            }
                            break;
                        }
                    break;
                case 0xF:
                    switch(NN){
                    case 0x0A:{
                        if(waiting_key == -1){
                            for(int i = 0; i<16; i++){
                                if(chip8.keypad[i]){
                                    waiting_key = i;
                                    break;
                                }
                            }
                            chip8.pc -= 2;
                        }
                        else{
                            if(chip8.keypad[waiting_key]){
                                chip8.pc -=2;
                            }
                            else{
                                chip8.V[X] = waiting_key;
                                waiting_key = -1;
                            }
                        }
                    }
                        break;
                    case 0x07:
                        chip8.V[X] = chip8.delay_timer;
                        break;
                    case 0x15:
                        chip8.delay_timer = chip8.V[X];
                        break;
                    case 0x18:
                        chip8.sound_timer = chip8.V[X];
                        break;
                    case 0x1E:
                        chip8.I += chip8.V[X];
                        break;
                    case 0x29:
                        chip8.I = 0x050 + chip8.V[X] * 5;
                        break;
                    case 0x33:
                        chip8.memory[chip8.I] = chip8.V[X]/100;
                        chip8.memory[chip8.I+1] = (chip8.V[X]/10)%10;
                        chip8.memory[chip8.I+2] = chip8.V[X]%10;
                        break;
                    case 0x55:
                        for (int i = 0; i <= X; i++){
                            chip8.memory[chip8.I+i] = chip8.V[i];
                        }
                        chip8.I += X+1;
                        break;
                    case 0x65:
                        for(int i = 0; i<= X; i++){
                            chip8.V[i] = chip8.memory[chip8.I+i];
                        }
                        chip8.I += X+1;
                        break;
                    }
                    break;

            }
    }
    int main()
    {
        InitWindow(64 * SCALE, 32 * SCALE, "Chip8");
        chip8.pc = 0x200;
        SetTargetFPS(60);
        load_rom("flightrunner.ch8");
        for(int i = 0; i<80; i++){
            chip8.memory[0x050+i] = font[i];
        }
        while (!WindowShouldClose()){
            update_keypad();
            screen_drawn = false;
            for(int i = 0; i < 10; i ++){
                emulate_cycle();
                if(screen_drawn){
                    break;
                }
            }
            if(chip8.delay_timer > 0){
                chip8.delay_timer--;
            }
            if(chip8.sound_timer > 0){
                chip8.sound_timer--;
            }
            BeginDrawing();
            ClearBackground(BLUE);
            draw_display();
            EndDrawing();
        }
        CloseWindow();
        return 0;
    }
