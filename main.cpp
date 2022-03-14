#include <iostream>
#include <SDL.h>
#include <SDL_video.h>
#include <thread>
#include <unistd.h>
#include <stack>
#include <random>

#define RAM_SIZE 4096
#define REG_SIZE 16
#define COSMAC_VIP // Changes jump offset instruction


#define first_nibble(x)((x & 0xf000u) >> 8u)

// Second nibble (also X) is used to look up one of the 16 registers, from V0 through VF
#define second_nibble(x)((x & 0x0f00u) >> 8u)
// The third nibble. Also used to look up one of the 16 registers (VY) from V0 through VF.
#define third_nibble(x)((x & 0x00f0u) >> 4u)
// The fourth nibble.
#define fourth_nibble(x)(x & 0x000fu)
// The second byte (third and fourth nibbles). An 8-bit immediate number.
#define second_byte(x)(x & 0x00ffu)
// The second, third and fourth nibbles. A 12-bit immediate memory address.
#define second_third_fourth_nibble(x)(x & 0x0fffu)

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

class Chip8 {
public:
    uint8_t memory[RAM_SIZE];
    uint8_t *pc;
    std::stack<uint8_t *> stack;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t registers[REG_SIZE];
    uint16_t *index_register;
    uint16_t instruction;
    uint16_t F;
    uint16_t X;
    uint16_t Y;
    uint16_t N;
    uint16_t N2;
    uint16_t N3;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Event event;

    Chip8();

    void present_window();

    void display();

    void clear_window();

    void draw_pixel(int x, int y);

    void read_instruction();

    void jump();

    void subroutine();

    void subroutine_return();

    void set_register();

    void add_register();

    void and_register();

    void xor_register();

    void shift_register(uint8_t direction);

    void set_index();

    void jump_with_offset();

    void subtract_register(uint8_t direction);

    void or_register();

    void random_register();

    void font_character();

};

void test_instruction();

int main() {
    Chip8 emulator;
    emulator.present_window();
    test_instruction();

    while (true) {
        if (SDL_PollEvent(&emulator.event)) {
            if (emulator.event.type == SDL_QUIT) {
                break;
            }
        }
        emulator.read_instruction();
        switch (emulator.F) {
            case 0x0:
                if (emulator.N2 == 0xEE) {
                    emulator.subroutine_return();
                } else if (emulator.N2 == 0xE0) {
                    emulator.clear_window();
                }
                break;
            case 0x1:
                emulator.jump();
                break;
            case 0x2:
                emulator.subroutine();
                break;
            case 0x6:
                emulator.set_register();
                break;
            case 0x7:
                break;
            case 0x8:
                switch (emulator.N) {
                    case 0x0:
                        emulator.set_register();
                        break;
                    case 0x1:
                        emulator.or_register();
                        break;
                    case 0x2:
                        emulator.and_register();
                        break;
                    case 0x3:
                        emulator.xor_register();
                        break;
                    case 0x4:
                        emulator.add_register();
                        break;
                    case 0x5:
                        emulator.subtract_register(0);
                        break;
                    case 0x7:
                        emulator.subtract_register(1);
                        break;
                    case 0x6:
                        emulator.shift_register(0);
                        break;
                    case 0xE:
                        emulator.shift_register(1);
                        break;
                    default:
                        printf("Unknown instruction in 0x8\n");
                }
            case 0xA:
                emulator.set_index();
                break;
            case 0xB:
                emulator.jump_with_offset();
                break;
            case 0xC:
                emulator.random_register();
                break;
            case 0xD:
                break;
            default:
                break;
                //printf("Unknown operand\n");
        }

        usleep(1);
    }

    return 0;
}

Chip8::Chip8(void) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(600, 600, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

    memcpy(memory, font, 80);
    pc = &memory[512];
}

void Chip8::display() {
    uint16_t x_cord = registers[X] % 64;
    uint16_t y_cord = registers[Y] % 32;
    registers[0xF] = 0;

    for (int i = 0; i < N; i++) {
        uint8_t sprite = *(index_register + i);
        for (int j = 0; j < 8; j++) {

        }

    }
}

void Chip8::present_window() {
    SDL_RenderPresent(renderer);
}

void Chip8::clear_window() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_Rect rect;
    rect.h = 600;
    rect.w = 600;
    SDL_RenderDrawRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
}

void Chip8::draw_pixel(int x, int y) {
    SDL_RenderDrawPoint(renderer, x, y);
}

void Chip8::read_instruction() {
    uint8_t first = *pc;
    uint8_t second = *(pc + 1);
    instruction = ((uint16_t) first << 8u) | second;
    if (pc == &memory[4096]) {
        printf("PC at end of memory region\n");
        exit(1);
    }
    pc += 2;
    F = first_nibble(instruction);
    X = second_nibble(instruction);
    Y = third_nibble(instruction);
    N = fourth_nibble(instruction);
    N2 = second_byte(instruction);
    N3 = second_third_fourth_nibble(instruction);
}

// 1NNN: Jump. This instruction should simply set PC to NNN
void Chip8::jump() {
    pc = &memory[N3];
}

// 2NNN calls the subroutine at memory location NNN.
void Chip8::subroutine() {
    stack.push(pc);
    pc = &memory[N3];
}

//void skip(Chip8 *emulator,)

void Chip8::subroutine_return() {
    pc = stack.top();
    stack.pop();
}

void Chip8::set_register() {
    registers[X] = N2;
}

void Chip8::or_register() {
    registers[X] |= registers[Y];
}

void Chip8::and_register() {
    registers[X] &= registers[Y];
}

void Chip8::xor_register() {
    registers[X] ^= registers[Y];
}

void Chip8::add_register() {
    uint16_t add_value = registers[X] + registers[Y];
    if (add_value > 255) {
        registers[0xF] = 1;
    }
    registers[X] = (uint8_t) add_value;
}

void Chip8::subtract_register(uint8_t direction) {
    uint16_t reg_1 = 0;
    uint16_t reg_2 = 0;
    if (direction == 0) {
        reg_1 = X;
        reg_2 = Y;
    } else if (direction == 1) {
        reg_1 = Y;
        reg_2 = X;
    }
    if (registers[reg_1] > registers[reg_2]) {
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }
    uint16_t subtract_value = registers[reg_1] - registers[reg_2];
    registers[X] = (uint8_t) subtract_value;
}

void Chip8::shift_register(uint8_t direction) {
    uint8_t bit_shifted;
    uint8_t value;
    registers[X] = registers[Y];
    bit_shifted = (registers[X] & 0x0001u);
    if (direction == 0) {
        registers[X] = registers[X] >> 1u;
    } else if (direction == 1) {
        registers[X] = registers[X] << 1u;
    }
    if (bit_shifted == 1) {
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }
}

void Chip8::set_index() {
    index_register = (uint16_t *) N3;
}

void Chip8::jump_with_offset() {
#ifdef COSMAC_VIP
    pc = (uint8_t *) (registers[0] + N3);
#endif
    pc = (uint8_t *) (registers[X] + N3);
}

void Chip8::random_register() {
    std::random_device r;
    std::default_random_engine engine(r());
    std::uniform_real_distribution<uint16_t> uniform_dist(0, SDL_MAX_UINT16);
    uint16_t random_value = uniform_dist(engine) & N2;
    registers[X] = random_value;
}

void Chip8::font_character() {
    index_register = (uint16_t *) &memory[X * 5];
}


void test_instruction() {
    Chip8 emulator;

    emulator.memory[512] = 0x12;
    emulator.memory[513] = 0x34;

    emulator.read_instruction();

    printf("Instruction: %x Second nibble:%x Third nibble:%x Fourth nibble:%x Second byte:%x Second,third,fourth byte:%x\n",
           emulator.instruction, emulator.X, emulator.Y, emulator.N, emulator.N2, emulator.N3);
}


