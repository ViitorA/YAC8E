#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ROM_START_ADDR 0x200
#define FONTSET_START_ADDR 0x50;
#define FONTSET_SIZE 80

typedef struct {
	uint8_t registers[16]; 	// 16 8-bit registers (v0 through vF)
	uint8_t memory[4096];  	// 4KB of RAM
	uint16_t index;		// Store memory addresses 
	uint16_t pc;		// Program Counter
	uint16_t stack[16];	//
	uint8_t sp;		// Stack Pointer
	uint8_t delayTimer; 	//
	uint8_t soundTimer;	//
	uint8_t keypad[16];	//
	uint32_t video[64 * 32];//
	uint16_t opcode;	//
} Chip8;

uint8_t fontset[FONTSET_SIZE] = {
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

void initChip8(Chip8 *c) {
	// Initialize PC
	c->pc = ROM_START_ADDR;

	// Load fonts into memory
	for(int i = 0; i < FONTSET_SIZE; i++) {
		c->memory[FONTSET_START_ADDR + i] = fontset[i];
	}
}

void loadROM(Chip8 *c, const char *filename) {
	FILE *rom_ptr = fopen(filename, "rb");
	
	if(rom_ptr == NULL) {
		printf("ERROR LOADING ROM");
		exit(EXIT_FAILURE);
	}

	fseek(rom_ptr, 0, SEEK_END); // vai até o fim do arquivo
	long rom_size = ftell(rom_ptr); // Pega o tamanho total
	rewind(rom_ptr);

	fread(&(c->memory[ROM_START_ADDR]), rom_size, 1, rom_ptr);

	fclose(rom_ptr);
}

/*
void readROM(Chip8 *c) {
	for(int i = ROM_START_ADDRESS; i < ROM_START_ADDRESS + rsize; i++) {
		printf("%02X ", c->memory[i]);
	}
}

int main() {
	Chip8 c;
	initChip8(&c);
	loadROM(&c, "teste.bin");

	exit(EXIT_SUCCESS);
}
*/
