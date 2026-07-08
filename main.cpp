#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

#include "SDL3/SDL.h"
#include "Chip8.h"

#define PIXEL_SIZE 20

SDL_Keycode keymap[16] = {
    SDLK_X,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_Q,
    SDLK_W,
    SDLK_E,
    SDLK_A,
    SDLK_S,
    SDLK_D,
    SDLK_Z,
    SDLK_C,
    SDLK_4,
    SDLK_R,
    SDLK_F,
    SDLK_V,
};

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cout << "Usage: chip8 <ROM file path>" << std::endl;
		return 1;
	}

	if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		std::cout << "Could not initialize SDL. Error: " << SDL_GetError();
		return 1;
	}
	
	Chip8 chip = Chip8();

	int width  = chip.DISPLAY_COLS * PIXEL_SIZE;
	int height = chip.DISPLAY_ROWS * PIXEL_SIZE;

	SDL_Window *win = SDL_CreateWindow("CHIP 8 EMULATOR", width, height, 0);

	if(win == NULL) {
		std::cout << "Window not initialized. Error: " << SDL_GetError();
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(win, NULL);	

	SDL_Texture* texture = SDL_CreateTexture(renderer, 
			SDL_PIXELFORMAT_ARGB8888, 
			SDL_TEXTUREACCESS_STREAMING, 
			chip.DISPLAY_COLS, chip.DISPLAY_ROWS);
	
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
	
	/* --- AUDIO --- */
	SDL_AudioSpec audio_spec;
	audio_spec.freq = 44100;            
	audio_spec.channels = 1; 
	audio_spec.format = SDL_AUDIO_S16;

	SDL_AudioStream* audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec, NULL, NULL);
	if (!audio_stream) {
	    std::cout << "Could not open audio stream. Error: " << SDL_GetError();
	    return 1;
	}
	SDL_ResumeAudioStreamDevice(audio_stream);

	int wave_phase = 0;

	// buffer
	uint32_t pixels[2048];

reload:
	if(!chip.load(argv[1]))
		return 1;

	uint64_t last_timer = SDL_GetTicks();

	while(true) {
		chip.cycle();
	
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_EVENT_QUIT) 
				goto clear_exit;
			
			if(e.type == SDL_EVENT_KEY_DOWN) {
				if(e.key.key == SDLK_ESCAPE) 
					goto clear_exit;

				if(e.key.key == SDLK_F1) { 
					chip.reset();
					goto reload;
				}
				
				for(int i = 0; i < 16; i++) {
					if(e.key.key == keymap[i]) {
						chip.keypad[i] = 1;
					}
				}
			}

			if(e.type == SDL_EVENT_KEY_UP) {
				for(int i = 0; i < 16; i++) {
					if(e.key.key == keymap[i]) {
						chip.keypad[i] = 0;
					}
				}
			}
		}
		
		/* --- TIMERS --- */
		uint64_t current_timer = SDL_GetTicks();
		if (current_timer - last_timer >= 16) {
		    chip.update_timers();
		    last_timer = current_timer;

		    if (chip.is_beeping()) {
			const int sample_rate = 44100;
			const int tone_hz = 440; 
			const int amplitude = 3000; 
			const int samples_per_frame = sample_rate / 60;
        
			int16_t audio_buffer[samples_per_frame];
			int half_period = sample_rate / tone_hz / 2;

			for (int i = 0; i < samples_per_frame; i++) {
			    // Alterna a onda entre o pico positivo e negativo de forma brusca (Onda Quadrada)
			    if ((wave_phase / half_period) % 2 == 0) {
				audio_buffer[i] = amplitude;
			    } else {
				audio_buffer[i] = -amplitude;
			    }
			    wave_phase++;
			}

			SDL_PutAudioStreamData(audio_stream, audio_buffer, sizeof(audio_buffer));
		    }
		}

		/* --- RENDERING --- */
		for (int i = 0; i < 2048; i++) {
		    // ARGB: 0xAARRGGBB. 
		    // display[i] == 1 => WHITE(0xFFFFFFFF)
		    // display[i] == 0 => BLACK(0xFF000000)
		    pixels[i] = chip.display[i] ? 0xFFFFFFFF : 0xFF000000;
		}

		SDL_UpdateTexture(texture, NULL, pixels, chip.DISPLAY_COLS * sizeof(uint32_t));

		SDL_RenderClear(renderer);
		SDL_RenderTexture(renderer, texture, NULL, NULL); // Nota: No SDL3 usa-se SDL_RenderTexture no lugar de SDL_RenderCopy
		SDL_RenderPresent(renderer);

		std::this_thread::sleep_for(std::chrono::microseconds(1200));
	}

clear_exit:
	SDL_DestroyAudioStream(audio_stream);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
