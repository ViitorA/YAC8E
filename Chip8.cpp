#include "Chip8.h"

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>

unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};


void Chip8::cycle() {
	// Fetch
	uint16_t opcode = (memory[PC] << 8) | (memory[PC + 1]);
	PC += 2;

	uint16_t addr = opcode & 0x0FFF; // nnn/addr: lowest 12b of the instruction    
	uint8_t n =  opcode & 0x000F; // n/nibble: lowest 4b of the instruction
	uint8_t kk = opcode & 0x00FF; // kk/byte: lowest 8b of the instruction
	uint8_t x = (opcode & 0x0F00) >> 8; // x: lower 4b of the high byte
	uint8_t y = (opcode & 0x00F0) >> 4; // y: upper 4b of the low byte

	// Execute
	switch(opcode & 0xF000) {
		case 0x0000: // 
			if (opcode == 0x00E0) { // 00E0 -> CLS 
				for (int i = 0; i < DISPLAY_ROWS * DISPLAY_COLS; i++) {
					display[i] = 0;		
				}
			} else if(opcode == 0x00EE) { // RET -> return from subroutine
				SP--;
				PC = stack[SP];
			}
			break;
		case 0x1000: // 1nnn -> JUMP addr
			PC = addr; 	
			break;
		case 0x2000: // 2nnn -> CALL addr
			stack[SP] = PC;
			SP++;
			PC = addr;
			break;
		case 0x3000: // 3xkk -> SE Vx, byte
			if (V[x] == kk) PC += 2;
			break;
		case 0x4000: // 4xkk -> SNE Vx, byte
			if (V[x] != kk) PC += 2;
			break;
		case 0x5000: // 5xy0 -> SE Vx, Vy
			if(V[x] == V[y]) PC += 2;
			break;
		case 0x6000: // 6xkk -> LD Vx, byte
			V[x] = kk;
			break;
		case 0x7000: // 7xkk -> ADD Vx, byte
			V[x] += kk;
			break;
		case 0x8000:
			switch(opcode & 0x000F) {
				case 0x0000: // LD Vx, Vy
					V[x] = V[y];
					break;
				case 0x0001: // OR Vx, Vy
					V[x] |= V[y];
					break;
				case 0x0002: // AND Vx, Vy
					V[x] &= V[y];
					break;
				case 0x0003: // XOR Vx, Vy
					V[x] ^= V[y];
					break;
				case 0x0004: { // ADD Vx, Vy
					uint16_t sum = V[x] + V[y];
					V[x] = sum & 0xFF;

					// flag carry
					V[0xF] = (sum > 0xFF) ? 1 : 0;
					break;
				}
				case 0x0005: // SUB Vx, Vy
					V[0xF] = (V[x] > V[y]) ? 1 : 0;
					V[x] = V[x] - V[y];
					break;
				case 0x0006: // SHR Vx {, Vy}
					V[0xF] = ((V[x] & 0x0001) == 1) ? 1 : 0;
					V[x] = V[x] >> 1;
					break;
				case 0x0007: // SUBN Vx, Vy
					V[0xF] = (V[y] > V[x]) ? 1 : 0;
					V[x] = V[y] - V[x];
					break;
				case 0x000E: // SHL Vx {,Vy}
					V[0xF] = ((V[x] & 0x80) == 1) ? 1 : 0;
					V[x] = V[x] << 1;
					break;
			}
			break;
		case 0x9000: { // 9xy0 -> SNE Vx, Vy
			if (V[x] != V[y]) PC += 2;
			break;
		}
		case 0xA000: // Annn -> LD I, addr
			I = addr;
			break;
		case 0xB000: // Bnnn -> JP V0, addr
			PC = addr + V[0];
			break;
		case 0xC000: { // Cxkk -> RND Vx, byte
			uint8_t random_byte = std::rand() % 256;
			V[x] = kk & random_byte;
			break;
		}
		case 0xD000: { // Dxyn -> DRW Vx, Vy, nibble
			uint8_t x_pos = V[x] % DISPLAY_COLS;
    			uint8_t y_pos = V[y] % DISPLAY_ROWS;
    			uint8_t height = n; 

    			V[0xF] = 0; 
		    	
			for (int row = 0; row < height; row++) {
       				uint8_t sprite_byte = memory[I + row];

        			for (int col = 0; col < 8; col++) {
				    uint8_t sprite_pixel = sprite_byte & (0x80 >> col);

				    int screen_x = x_pos + col;
				    int screen_y = y_pos + row;

				    if (screen_x >= DISPLAY_COLS || screen_y >= DISPLAY_ROWS) {
					continue;
				    }

				    // 2D -> 1D
				    int screen_index = screen_y * DISPLAY_COLS + screen_x;

				    if (sprite_pixel) {
					if (display[screen_index] == 1) {
					    V[0xF] = 1;
					}

					display[screen_index] ^= 1;
            			    }
        			}
    			}
    			break;
		}
		case 0xE000: 
			switch(opcode & 0x00FF) {
				case 0x009E: // Ex9E -> SKP Vx
					if(keypad[V[x]]) PC += 2;
					break;
				case 0x00A1: // ExA1 -> SKNP Vx
					if(!keypad[V[x]]) PC += 2;
					break;
			}
			break;
		case 0xF000:
			switch(opcode & 0x00FF) {
				case 0x0007: // Fx07 -> LD Vx, DT
					V[x] = delay_timer;
					break;
				case 0x000A: { // Fx0A -> LD Vx, K
					bool key_pressed = false;

					for(int i = 0; i < 16; i++) {
						if(keypad[i]) {
							V[x] = i;
							key_pressed = true;
							break;
						}
					}
					
					if(!key_pressed) {
						PC -= 2;
						return;
					}

					break;
				}
				case 0x0015: // Fx15 -> LD DT, Vx
					delay_timer = V[x];
					break;
				case 0x0018: // Fx18 -> LD ST, Vx
					sound_timer = V[x];
					break;
				case 0x001E: // Fx1E -> ADD I, Vx
					I += V[x];
					break;
				case 0x0029: // Fx29 -> LD F, Vx
					I = V[x] * 0x5;
					break;
				case 0x0033: // Fx33 -> LD B, Vx
					memory[I] = V[x] / 100;
					memory[I+1] = (V[x] / 10) % 10;
					memory[I+2] = V[x] % 10;
					break;
				case 0x0055: // Fx55 -> LD[I], Vx
					for(int i = 0; i <= x; i++) {
						memory[I + i] = V[i];		
					}
					break;
				case 0x0065: // Fx65 -> LD Vx, [I]
					for(int i = 0; i <= x; i++) {
						V[i] = memory[I + i];
					}
					break;
			}
			break;
	}
}

void Chip8::reset() {
		I = 0;
		PC = 0x200;
		SP = 0;

		delay_timer = 0;
		sound_timer = 0;

		std::memset(V, 0, sizeof(V));
		std::memset(memory, 0, sizeof(memory));
		std::memset(stack, 0, sizeof(stack));
		std::memset(display, 0, sizeof(display));
		std::memset(keypad, 0, sizeof(keypad));

		// Load fontset into memory
		for(int i = 0; i < 80; i++) {
			memory[i] = chip8_fontset[i];
		}
}

void Chip8::update_timers() {
	if(delay_timer > 0) delay_timer--;

	if(sound_timer > 0) sound_timer--;
}

bool Chip8::load(const char *file_path) {
	std::cout << "Loading ROM: " << file_path << std::endl;

	// std::ios::ate -> Abre o arquivo e posiciona no final (at the end)
	std::ifstream file(file_path, std::ios::binary | std::ios::ate);
	if(!file.is_open()) {
		std::cerr << "Unable to load ROM." << std::endl;
		return false;
	}

	std::streamsize rom_size = file.tellg();
	file.seekg(0, std::ios::beg);

	if(rom_size > (MEM_SIZE - 0x200)) {
		std::cerr << "ROM is too large to fit in memory." << std::endl;
		return false;
	}	

	// reinterpret_cast -> converte uint8_t* p/a char*, que é o tipo de entrada do file.read()
	if(file.read(reinterpret_cast<char*>(&memory[0x200]), rom_size)) {
		return true;
	} else {
		std::cerr << "Error reading ROM." << std::endl;
		return false;
	}
}

Chip8::Chip8() {
	reset();
}


