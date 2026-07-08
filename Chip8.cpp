#include "chip8.h"

#include <cstdint>
#include <fstream>
#include <iostream>

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
			uint8_t rand = rand() % 256;
			V[x] = kk & rand;
			break;
		}
		case 0xD000: // Dxyn -> DRW Vx, Vy, nibble TODO TODO TODO

		case 0xE000: 
			switch(opcode & 0x00FF) {
				case 0x009E: // Ex9E -> SKP Vx TODO TODO TODO
					
				case 0x00A1: // ExA1 -> SKNP Vx TODO TODO TODO


			}

		case 0xF000:
			switch(opcode & 0x00FF) {
				case 0x0007: // Fx07 -> LD Vx, DT
					V[x] = delay_timer;
					break;
				case 0x000A: // Fx0A -> LD Vx, K TODO TODO TODO
					// TODO
				case 0x0015: // Fx15 -> LD DT, Vx
					delay_timer = V[x];
					break;
				case 0x0018: // Fx18 -> LD ST, Vx
					sound_timer = V[x];
					break;
				case 0x001E: // Fx1E -> ADD I, Vx
					I += V[x];
					break;
				case 0x0029: // Fx29 -> LD F, Vx TODO TODO TODO
					// TODO
				case 0x0033: // Fx33 -> LD B, Vx TODO TODO TODO

				case 0x0055: // Fx55 -> LD[I], Vx TODO TODO TODO
					
				case 0x0065: // Fx65 -> LD Vx, [I] TODO TODO TODO
			}
			break;
	}
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
