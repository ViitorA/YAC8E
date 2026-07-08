#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>

class chip8 {
	private:
		static constexpr int MEM_SIZE = 4096;
		static constexpr int STACK_SIZE = 16;

		static constexpr int DISPLAY_ROWS = 32;
		static constexpr int DISPLAY_COLS = 64;
		
		uint16_t I = 0;
		uint16_t PC = 0x200;
		uint8_t SP = 0;
		uint8_t V[16] = {0};

		uint8_t delay_timer = 0;
		uint8_t sound_timer = 0;

		uint8_t memory[MEM_SIZE] = {0};
		uint16_t stack[STACK_SIZE] = {0};

		uint8_t display[DISPLAY_ROWS][DISPLAY_COLS] = {0};
	public:
		void cycle();
};

#endif
