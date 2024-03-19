#include "root.h"

struct Audio_Ring_Buffer {
    int size;
    int write_idx;
    int read_idx;
    void *data;
};

//global SDL_Renderer *renderer;
global SDL_Window *window;

internal void update_button_state(ButtonState *state, bool is_down) {
    state->ended_down = is_down;
    state->half_transition_count++;
}

bool handle_event(SDL_Event *event, GameInput *game_input) {
    //TODO: Handle rebinding keys
    bool should_quit = false;

    GameController *keyboard_controller = &game_input->controllers[0];
    
    switch (event->type) {
        case SDL_QUIT: {
            should_quit = true;
        } break;
        case SDL_MOUSEMOTION: {
            game_input->mouse_pos.x = event->motion.x;
            game_input->mouse_pos.y = event->motion.y;
            game_input->mouse_delta.x = event->motion.xrel;
            game_input->mouse_delta.y = event->motion.yrel;
            
        } break;
        case SDL_MOUSEBUTTONDOWN: 
        case SDL_MOUSEBUTTONUP: {
            bool is_down = event->button.state == SDL_PRESSED;
            
            switch (event->button.button) {
                case SDL_BUTTON_LEFT: {
                    update_button_state(&game_input->mouse_left, is_down);
                } break;
                case SDL_BUTTON_RIGHT: {
                    update_button_state(&game_input->mouse_right, is_down);
                } break;
            }
        } break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            //bool was_down = false;
            bool is_down = event->key.state == SDL_PRESSED;
            
            SDL_Keycode key_code = event->key.keysym.sym;
            if (event->key.repeat == 0) {
                switch (key_code) {
                    case SDLK_w: {
                        update_button_state(&keyboard_controller->up, is_down);
                    } break;
                    case SDLK_a: {
                        update_button_state(&keyboard_controller->left, is_down);
                    } break;
                    case SDLK_s: {
                        update_button_state(&keyboard_controller->down, is_down);
                    } break;
                    case SDLK_d: {
                        update_button_state(&keyboard_controller->right, is_down);
                    } break;
                    case SDLK_q: {
                        update_button_state(&keyboard_controller->draw_toggle, is_down);
                    } break;
                    default: break;
                }
            }
    	} break;
        
        default: break;
    }
    
    return should_quit;
}

/* This function should arguably be split - loading the files is the platform
 * layer's or even the game's concern, whereas turning it into the renderer's 
 * texture representation is the renderer's concern. 
 */
TextureHandle load_texture(char *filename) {
    /*
    TextureHandle result = {};

	int required_components = STBI_rgb_alpha;
	int x, y, num_components;
	// TODO: Define STBI_MALLOC etc to use our allocated game memory
    unsigned char *data = stbi_load(filename, &x, &y, &num_components, required_components);

    if (data == NULL) {
        printf("Could not load file: %s\n", filename); //TODO: logging
		return result;
    }

    int depth = 32;
    int pitch = 4*x; // 4 for rgba
    int pixel_format = SDL_PIXELFORMAT_RGBA32;

    SDL_Surface *surface = 
    	SDL_CreateRGBSurfaceWithFormatFrom((void *)data, x, y, depth, pitch, pixel_format);

    if (surface == NULL) {
        printf("Could not create SDL surface. Error: %s\n", SDL_GetError()); //TODO: logging
		stbi_image_free(data);	
		return result;
    }
   
    result.sdl_texture = SDL_CreateTextureFromSurface(renderer, surface);
    result.width  = x;
    result.height = y;

    if (result.sdl_texture == NULL) {        
        printf("Could not create texture. Error %s\n", SDL_GetError()); //TODO: logging
		stbi_image_free(data);
        return result;
    }

    printf("Loaded texture %s\n", filename); //TODO: logging

    SDL_FreeSurface(surface);
    stbi_image_free(data);
    
    return result;
    */

    assert(0);
}

void audio_callback(void *userdata, u8 *audio_buffer, int length) {
    Audio_Ring_Buffer *ring_buffer = (Audio_Ring_Buffer*)userdata;

    int region_1_size = length;
    int region_2_size = 0;
    if (ring_buffer->read_idx + length > ring_buffer->size) {
        region_1_size = ring_buffer->size - ring_buffer->read_idx;
        region_2_size = length - region_1_size;
    }
    
    memcpy(audio_buffer, (uint8*)(ring_buffer->data) + ring_buffer->read_idx, region_1_size);
    memcpy(&audio_buffer[region_1_size], ring_buffer->data, region_2_size);
    ring_buffer->read_idx = (ring_buffer->read_idx + length) % ring_buffer->size;
    ring_buffer->write_idx = (ring_buffer->read_idx + 2048) % ring_buffer->size;
}

internal int get_window_refresh_rate(SDL_Window *window) {
    SDL_DisplayMode display_mode;
    int display_index = SDL_GetWindowDisplayIndex(window);
    int default_refresh_rate = 60;

    if (SDL_GetDesktopDisplayMode(display_index, &display_mode) != 0) {
		return default_refresh_rate;
   	}

   	if (display_mode.refresh_rate == 0) {
       	return default_refresh_rate;
   	}

   	return display_mode.refresh_rate;
}

internal inline f32 get_seconds_elapsed(u64 old_counter, u64 new_counter) {
    return ((f32)(new_counter - old_counter) / (f32)(SDL_GetPerformanceFrequency()));
}

int main(int argc, char *argv[]) {
    
    int window_flags = 0;
    #ifdef OPENGL
    window_flags |= SDL_WINDOW_OPENGL;
    #endif
    #ifdef METAL
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
    #endif
    int window_width = 960;
    int window_height = 540;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Can't initiate SDL. Error: %s\n", SDL_GetError()); // TODO: logging
        return 1;
    }

    window = SDL_CreateWindow("game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, window_flags);
    
    if (!window) { 
        printf("Can't open window. Error: %s\n", SDL_GetError()); // TODO: logging
        return 1;
    }

    // OpenGL setup
    #ifdef OPENGL
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    
    SDL_GLContext gl_context = SDL_GL_CreateContext(window); 
    if (!gl_context) { 
        printf("Failed to create an OpenGL context. Error: %s\n", SDL_GetError()); // TODO: logging
        return 1;
    }

    gladLoadGLLoader(SDL_GL_GetProcAddress);

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    printf("Initialized OpenGL %i.%i\n", major, minor);
    #endif

    // sound initialization
    int samples_per_second = 44100;
    int bytes_per_sample = sizeof(s16) * 2;
    int ring_buffer_size = samples_per_second * bytes_per_sample;
    
    Audio_Ring_Buffer audio_ring_buffer = {
       .size = ring_buffer_size,
       .write_idx = 0,
       .read_idx = 0,
       .data = malloc(ring_buffer_size),
    };
    
    SDL_AudioSpec audio_spec_requested = {};
    SDL_AudioSpec audio_spec = {};

    audio_spec_requested.freq      = samples_per_second;
    audio_spec_requested.format    = AUDIO_S16LSB;
    audio_spec_requested.channels  = 2;
    audio_spec_requested.samples   = 1024; // NOTE: should be power of 2.
    audio_spec_requested.callback  = audio_callback;
    audio_spec_requested.userdata  = (void *)&audio_ring_buffer;

    SDL_AudioDeviceID audio_device_id = SDL_OpenAudioDevice(
        NULL, 0,
        &audio_spec_requested, &audio_spec,
        SDL_AUDIO_ALLOW_FORMAT_CHANGE
    );

    if (!audio_device_id) {
        printf("Failed to create an Audio device. Error: %s\n", SDL_GetError()); // TODO: logging
        return 1;
    }

    //SDL_PauseAudioDevice(audio_device_id, 0);

	// allocate game memory
	GameMemory game_memory = {};

	game_memory.permanent_storage_size = MEGABYTES(64);
	game_memory.temporary_storage_size = MEGABYTES(64);
	u64 total_memory_size = game_memory.permanent_storage_size + game_memory.temporary_storage_size;

	game_memory.permanent_storage = calloc(1, total_memory_size);
	game_memory.temporary_storage = (u8*)(game_memory.permanent_storage) + game_memory.permanent_storage_size;

	if (!game_memory.permanent_storage || !game_memory.temporary_storage) {
        printf("Failed to allocate game memory.\n"); // TODO: logging
        return 1;
	}

	// Set up input interface
	// TODO: handle controller support
	GameInput input = {};
    SDL_SetRelativeMouseMode(SDL_TRUE); // hide cursor and lock it to the window

	// timing initialization
	// TODO: make game_update_hz be multiple of screen_hz
    int screen_hz = get_window_refresh_rate(window);
    int game_update_hz = 60;
    f32 target_seconds_per_frame = 1.0f / (f32)game_update_hz;

    u64 prev_frame_counter = SDL_GetPerformanceCounter();
    bool should_quit = false;
    while (!should_quit) {
		u64 frame_counter = SDL_GetPerformanceCounter();
		f32 frame_time = get_seconds_elapsed(prev_frame_counter, frame_counter);
		prev_frame_counter = frame_counter;

        // reset button transitions
		for (int i = 0; i < ARRAY_COUNT(input.controllers); i++) {
            for (int button_idx = 0; 
                 button_idx < ARRAY_COUNT(input.controllers[i].buttons); 
                 button_idx++) {
    			input.controllers[i].buttons[button_idx].half_transition_count = 0;
            }
		}
		// reset mouse delta
		input.mouse_delta = {0};
		
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            should_quit = handle_event(&event, &input);
        }

        // fill test sound buffer
        SDL_LockAudioDevice(audio_device_id);
        
        SDL_UnlockAudioDevice(audio_device_id); 

        Vec2 window_dims = {(f32)window_width, (f32)window_height};
        game_update_and_render(&game_memory, &input, window_dims, frame_time);

        SDL_GL_SwapWindow(window);

		// Enforce frame rate by waiting for any remaining time
		if (get_seconds_elapsed(frame_counter, SDL_GetPerformanceCounter()) 
		      < target_seconds_per_frame) {
    		u32 delay_time = 1000 * (target_seconds_per_frame - get_seconds_elapsed(frame_counter, SDL_GetPerformanceCounter())) - 1;
    		SDL_Delay(delay_time);
			// Spin the last millisecond to reduce SDL_Delay overshooting.
    		while (get_seconds_elapsed(frame_counter, SDL_GetPerformanceCounter()) 
    		       < target_seconds_per_frame) {
        		// wait
    		}
		}

#if PERF_DIAGNOSTIC
		// Performance measurements
		f32 seconds_elapsed = get_seconds_elapsed(frame_counter, SDL_GetPerformanceCounter());
		f32 ms_per_frame = 1000.0f * seconds_elapsed;
		f32 fps = 1.0f / seconds_elapsed;

		printf("%.02f ms/frame, %.02f fps\n", ms_per_frame, fps);
#endif
    }

    SDL_CloseAudioDevice(audio_device_id);
    SDL_Quit();
    return 0;
}
