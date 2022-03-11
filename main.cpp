#include <iostream>
#include <SDL.h>
#include <SDL_video.h>
#include <thread>
#include <unistd.h>
#include <stack>

#define RAM_SIZE 4096


// Second nibble (also X) is used to look up one of the 16 registers, from V0 through VF
#define X(x)((x & 0x0f00u) >> 8)
// The third nibble. Also used to look up one of the 16 registers (VY) from V0 through VF.
#define Y(x)((x & 0x00f0u) >> 4)
// The fourth nibble.
#define N(x)(x & 0x000fu)
// The second byte (third and fourth nibbles). An 8-bit immediate number.
#define NN(x)(x & 0x00ffu)
// The second, third and fourth nibbles. A 12-bit immediate memory address.
#define NNN(x)(x & 0x0fffu)

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

typedef struct {
    uint8_t memory[RAM_SIZE];
    uint8_t *pc;
    std::stack<uint8_t*> stack;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t registers[16];
} Chip8;

SDL_Renderer *setup_renderer();

void draw_pixel(SDL_Renderer *renderer, int x, int y);

void update_window(SDL_Renderer *renderer);

void clear_window(SDL_Renderer *renderer);

void emulate(SDL_Renderer *renderer);

void set_font(Chip8 *emulator);

void set_pc(Chip8 *emulator);

uint16_t read_instruction(Chip8 *emulator);

void test_instruction();

void jump(Chip8 *emulator, uint16_t mem_location);

void subroutine(Chip8 *emulator, uint16_t mem_location);

void subroutine_return(Chip8 *emulator);


int main() {
    SDL_Renderer *renderer;
    renderer = setup_renderer();
    SDL_Event event;
    SDL_RenderPresent(renderer);
    Chip8 emulator = {0};
    set_font(&emulator);
    set_pc(&emulator);
    test_instruction();

    while (true) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                break;
            }
        }
        uint16_t instruction = read_instruction(&emulator);
        uint16_t second_nibble = X(instruction);
        uint16_t third_nibble = Y(instruction);
        uint16_t fourth_nibble = N(instruction);
        uint16_t second_byte = NN(instruction);
        uint16_t second_third_fourth_nibble = NNN(instruction);
        switch (instruction & 0x000fu) {
            case 0x0:
                if (second_byte == 0xEE) {
                    subroutine_return(&emulator);
                } else if (second_byte == 0xE0) {
                    clear_window(renderer);
                }
                break;
            case 0x1:
                jump(&emulator, second_third_fourth_nibble);
                break;
            case 0x2:
                subroutine(&emulator, second_third_fourth_nibble);
                break;
            case 0x6:
                break;
            case 0x7:
                break;
            case 0xA:
                break;
            case 0xD:
                break;
            default:
                printf("Unknown operand\n");
        }

        usleep(1);
    }

    return 0;
}

SDL_Renderer *setup_renderer() {
    SDL_Renderer *renderer;
    SDL_Window *window;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(600, 600, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    return renderer;
}

void update_window(SDL_Renderer *renderer) {
    SDL_RenderPresent(renderer);
}

void clear_window(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_Rect rect;
    rect.h = 600;
    rect.w = 600;
    SDL_RenderDrawRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
}

void draw_pixel(SDL_Renderer *renderer, int x, int y) {
    SDL_RenderDrawPoint(renderer, x, y);
}

void set_font(Chip8 *emulator) {
    memcpy(emulator->memory, font, 80);
}

void set_pc(Chip8 *emulator) {
    emulator->pc = &emulator->memory[512];
}
uint16_t read_instruction(Chip8 *emulator) {
    uint16_t instruction = 0;
    uint8_t first = *emulator->pc;
    uint8_t second = *(emulator->pc + 1);
    instruction = ((uint16_t ) first << 8u) | second;
    if (emulator->pc == &emulator->memory[4096]) {
        printf("PC at end of memory region");
        exit(1);
    }
    emulator->pc += 2;
    return instruction;
}

void jump(Chip8 *emulator, uint16_t mem_location) {
    emulator->pc = &emulator->memory[mem_location];
}

void subroutine(Chip8 *emulator, uint16_t mem_location) {
    emulator->stack.push(emulator->pc);
    emulator->pc = &emulator->memory[mem_location];
}

void subroutine_return(Chip8 *emulator) {
    emulator->pc = emulator->stack.top();
    emulator->stack.pop();
}
void test_instruction() {
    Chip8 emulator = {0};
    set_font(&emulator);
    set_pc(&emulator);

    emulator.memory[512] = 0x12;
    emulator.memory[513] = 0x34;

    uint16_t instruction = read_instruction(&emulator);
    printf("%x\n", instruction);
    uint16_t second_nibble = X(instruction);
    uint16_t third_nibble = Y(instruction);
    uint16_t fourth_nibble = N(instruction);
    uint16_t second_byte = NN(instruction);
    uint16_t second_third_fourth_nibble = NNN(instruction);
    printf("Second nibble:%x Third nibble:%x Fourth nibble:%x Second byte:%x Second,third,fourth byte:%x", second_nibble, third_nibble, fourth_nibble, second_byte, second_third_fourth_nibble);
}


