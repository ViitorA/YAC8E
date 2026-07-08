#include <iostream>
#include <cstdint>

#include "SDL3/SDL.h"
#include "Chip8.h"

#define PIXEL_SIZE 5

uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cout << "Usage: chip8 <ROM file path>" << std::endl;
		return 1;
	}

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::printf("Could not initialize SDL. Error: %s\n", SDL_GetError());
		return 1;
	}
	
	Chip8 chip = Chip8();

	int width  = chip.DISPLAY_COLS * PIXEL_SIZE;
	int height = chip.DISPLAY_ROWS * PIXEL_SIZE;

	SDL_Window *win = SDL_CreateWindow("CHIP 8 EMULATOR", width, height);

	if(win == NULL) {
		std::printf("Window not initialized. Error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(win, NULL);	

	SDL_Texture* texture = SDL_CreateTexture(renderer, 
			SDL_PIXELFORMAT_ARGB8888, 
			SDL_TEXTUREACCESS_STREAMING, 
			chip.DISPLAY_COLS, chip.DISPLAY_ROWS);

	// buffer
	uint32_t pixels[2048];

load:
	if(!chip.load(argv[1]))
		return 1;

	while(true) {
		chip.cycle();
	
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT) 
				goto saida;
			
			if(e.type == SDL_KEYDOWN) {
				if(e.key.keysym.sym == SDLK_ESCAPE) 
					goto saida;

				if(e.key.keysym.sym == SDLK_F1) 
					goto load;
				
				for(int i = 0; i < 16; i++) {
					if(e.key.keysym.sym == keymap[i]) {
						chip8.key[i] = 1;
					}
				}
			}

			if(e.type == SDL_KEYUP) {
				for(int i = 0; i < 16; i++) {
					if(e.key.keysym.sym == keymap[i]) {
						chip8.key[i] = 0;
					}
				}
			}
		}



		std::this_thread::sleep_for(std::chrono::microseconds(1200));
	}

saida:
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
