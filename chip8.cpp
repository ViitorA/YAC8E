#include "chip8.h"

#include <cstdint>

void chip8::cycle() {
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
		case 0x0000:
			if (opcode == 0x00E0) { // CLS -> clear display
				// TODO
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
					// TODO: FAZER O MECANISMO DE CARRY
					//chip->V[x] += chip->V[y];
					if (V[vx_i] & 0x
							// Como verificar se o resultado deu +8 bits?

					break;
				}
				case 0x0005:
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
		case 0xC000:
			
		case 0xD000:

		case 0xE000:

		case 0xF000:
	}
}
