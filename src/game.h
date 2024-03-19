#ifndef GAME_H
#define GAME_H

struct GameMemory {
	bool is_initialized;
    u64 permanent_storage_size;
	u64 temporary_storage_size;
    void *permanent_storage;
    void *temporary_storage;
};

enum EntityType {
    entity_type_player,
    entity_type_wall,
    entity_type_frog,

    entity_type_count    
};

enum EntityState {
    entity_state_frog_idle,
    entity_state_frog_jumping,

    entity_state_count
};

struct Entity {
    bool exists;
    EntityType type;
    EntityState state;
    Vec3 p; // m, position
    Vec3 dp; // m / s, velocity 
    f32 mass; //kg
    Vec2 collide_rect_size;
    Vec3 color;
    f32 idle_secs;
};

struct Animation {
    f32 seconds_per_frame;
    f32 elapsed_seconds;
    Ivec2 current_frame;
    Ivec2 num_frames;
    bool is_flipped;

    /* the center point for drawing might be offset from 
     * the center point of the texture. TODO: is this the best place to store this? */
    Vec2 center_offset; 
};

struct Camera {
    Vec3 position;
    f32 pitch; // should stay between +-pi/2 radians (+-90 degrees)
    f32 yaw;   // should stay between +-2pi radians (+-360 degrees)
};

struct GameState {
    Entity entities[256];
    u32 max_entity_count;
    u32 entity_count;
    Camera camera;
    MemoryArena ground_arena;
    Mesh ground_mesh;
    TextureHandle player_texture;
    TextureHandle frog_sit_texture;
    TextureHandle frog_jump_texture;
    u32 random_state;
};

struct ButtonState {
    int half_transition_count;
    bool ended_down;
};

// TODO: add controller support
struct GameController {

	union {
    	ButtonState buttons[5];
    	struct {
        	ButtonState up;
        	ButtonState down;
        	ButtonState left;
        	ButtonState right;
        	ButtonState draw_toggle;
    	};
	};
};

struct GameInput {
   	Vec2 mouse_pos; //TODO: Do OSes support multiple mice?
   	Vec2 mouse_delta;
   	ButtonState mouse_left, mouse_right;
   	
	GameController controllers[5]; // element 0 is the keyboard
};

internal void game_update_and_render(GameMemory *game_memory, GameInput *game_input, Vec2 window_dims, f32 frame_dt);

#endif //GAME_H
