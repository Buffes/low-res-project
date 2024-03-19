#pragma once
#include "root.h"
#include "game.h"

/*
 * TODO list:
 ***** sound
 * options/config file
 * dynamic resize of the screen (low res) buffer for debugging/testing
 * rebinding keys
 * better movement
 * basic physics (?)
 * terrain
 */

#define FROG_AVOIDANCE_THRESHOLD 100.0f // meters
#define MOUSE_SENSITIVITY 14.0f
#define MOVE_SPEED 10.0f


internal bool AABB_intersects(Vec2 first_center, Vec2 first_size, 
                              Vec2 second_center, Vec2 second_size) {
    Vec2 first_min = first_center - first_size / 2;
    Vec2 first_max = first_center + first_size / 2;
    Vec2 second_min = second_center - second_size / 2;
    Vec2 second_max = second_center + second_size / 2;
    // printf("first: %f, %f, %f, %f\n", first_min.x, first_min.y, first_max.x, first_max.y);
    // printf("second: %f, %f, %f, %f\n", second_min.x, second_min.y, second_max.x, second_max.y);
    if (first_min.x < second_max.x && first_max.x > second_min.x) {
        if (first_min.y < second_max.y && first_max.y > second_min.y) {
            return true;
        }
    }
    
    return false;
}


internal Entity *get_entity(GameState *game_state, u32 entity_index) {
    assert(entity_index < game_state->max_entity_count);
    Entity *entity = &game_state->entities[entity_index];
    return entity;
}

internal u32 add_entity(GameState *game_state) { 
    assert(game_state->entity_count < game_state->max_entity_count);
    
    u32 entity_index = game_state->entity_count;
    game_state->entities[entity_index] = {};
    game_state->entities[entity_index].exists = true;
    game_state->entity_count++;
    
    return entity_index;
}

internal u32 add_player(GameState *game_state, Vec3 position,
                        Vec2 collide_rect_size) {
    u32 entity_index = add_entity(game_state);
    Entity *player = get_entity(game_state, entity_index);

	player->p = position;
	player->collide_rect_size = collide_rect_size;
	player->mass = 1.0f;

    return entity_index;
}

internal u32 add_wall(GameState *game_state, Vec3 position, Vec2 size, Vec3 color) {
    u32 entity_index = add_entity(game_state);
    Entity *wall = get_entity(game_state, entity_index);

    wall->type = entity_type_wall;
    wall->p = position;
    wall->collide_rect_size = size;
    wall->color = color;
	wall->mass = 1.0f;

    return entity_index;
}

internal u32 add_frog(GameState *game_state, Vec3 position, Vec2 size) {
    u32 entity_index = add_entity(game_state);
    Entity *frog = get_entity(game_state, entity_index);

    frog->type = entity_type_frog;
    frog->p = position;
    frog->collide_rect_size = size;
    frog->mass = 1.0f;

    return entity_index;
}

internal Mesh fill_ground_mesh(MemoryArena *vertex_arena) {
    s32 x_len = 100;
    s32 z_len = 100;

    u32 num_vertices  = (u32)(x_len * z_len);
    u32 num_triangles = (u32)(2 * (x_len - 1) * (z_len - 1));
    u32 num_indices   = 3 * num_triangles;
    
    Mesh ground = {
        .num_vertices = num_vertices,
        .num_indices = num_indices,
        .vertices = reserve_array(vertex_arena, Vec3, num_vertices),
        .indices = reserve_array(vertex_arena, u32, num_indices)
    };

    // set vertex positions
    for (s32 x = 0; x < x_len; x++) {
        for (s32 z = 0; z < z_len; z++) {
            ground.vertices[z * x_len + x] = {(f32)x, -2.0f, (f32)z};
        } 
    }

    // set indices.
    for (s32 z = 0; z < z_len - 1; z++) {
        for (s32 x = 0; x < x_len - 1; x++) {
            s32 quad_start_idx = 6 * (z * (x_len - 1) + x);
            // top left triangle
            ground.indices[quad_start_idx]     = z * x_len + x; 
            ground.indices[quad_start_idx + 1] = (z + 1) * x_len + x; 
            ground.indices[quad_start_idx + 2] = z * x_len + x + 1;           
           
            // bottom right triangle
            ground.indices[quad_start_idx + 3] = (z + 1) * x_len + x + 1;
            ground.indices[quad_start_idx + 4] = z * x_len + x + 1;
            ground.indices[quad_start_idx + 5] = (z + 1) * x_len + x;
        } 
    }
    
    return ground;
}

internal inline bool button_was_released(ButtonState button) {
    return !button.ended_down && button.half_transition_count > 0;
}

bool should_draw = true; // NOTE: Just for debugging

internal void game_update_and_render(GameMemory *game_memory, GameInput *game_input, Vec2 window_dims, f32 frame_dt) {
    GameState *game_state = (GameState*)game_memory->permanent_storage;
    u32 seed = sizeof(GameState); // TODO: Change to something else
    persistent Mesh ground; // TODO: Move to gamestate
    
	if (!game_memory->is_initialized) {
    	setup_render_state();
    	
        MemoryArena mesh_arena = {
            .size = game_memory->permanent_storage_size - sizeof(GameState),
            .used = 0,
            .start = (u8*)game_memory->permanent_storage + sizeof(GameState)
        };
        
    	ground = fill_ground_mesh(&mesh_arena);
        game_memory->is_initialized = true;
    	/*
    	game_state->max_entity_count = ARRAY_COUNT(game_state->entities);
        game_state->player_texture = load_texture("../assets/player_sheet.png");
        game_state->frog_sit_texture = load_texture("../assets/frog_sit.png");
        game_state->frog_jump_texture = load_texture("../assets/frog_jump.png");
        game_state->random_state = 1337;

        // TODO: This is kinda dumb and should probably just be a 
        // world space -> screen space conversion in a "proper" game
        f32 level_width =  window_dims.x;
        f32 level_height = window_dims.y;
        f32 wall_size = 20.0f; // pixels
        f32 half_wall_size = wall_size / 2;
        Vec3 wall_color = Vec3{0.3f, 0.3f, 0.3f};
        f32 half_level_w = level_width  / 2;
        f32 half_level_h = level_height / 2;
        
        add_player(game_state, Vec3{half_level_w, half_level_h, 0.0f}, Vec2{32, 80});

        // Left
        add_wall(game_state, Vec3{half_wall_size, half_level_h, 0.0f}, Vec2{wall_size, level_width}, wall_color);

        // Right
        add_wall(game_state, Vec3{level_width - half_wall_size, half_level_h, 0.0f}, Vec2{wall_size, level_width}, wall_color);

        // Top
        add_wall(game_state, Vec3{half_level_w, half_wall_size, 0.0f}, Vec2{level_width, wall_size}, wall_color);

    	// Bottom
        add_wall(game_state, Vec3{half_level_w, level_height - half_wall_size, 0.0f}, Vec2{level_width, wall_size}, wall_color);

        // add_wall(game_state, Vec2{150, 300}, Vec2{100, 240}, Vec3{0.0f, 0.3f, 0.1f});

        for (int i = 0; i < 8; i++) {
            Vec3 pos = {
                random_f32(&game_state->random_state, 0.0f, level_width),
                random_f32(&game_state->random_state, 0.0f, level_height),
                0.0f
            };

            add_frog(game_state, pos, Vec2{16, 8});
        }
    	
    	// Animation properties could be read from a file
    	player_anim->seconds_per_frame = 0.2f;
        player_anim->num_frames = Ivec2{2, 1};
        player_anim->center_offset = {-10.0f, -2.0f};
        
        */
	}

	// move player and camera based on input
    Vec2 head_turn_delta = -game_input->mouse_delta;
    head_turn_delta.x /= window_dims.x;
    head_turn_delta.y /= window_dims.y;
    head_turn_delta *= MOUSE_SENSITIVITY;
    Camera *camera = &game_state->camera;
    camera->pitch += head_turn_delta.y;
	camera->pitch = clamp(game_state->camera.pitch, -PI_HALF, PI_HALF); 
    camera->yaw += head_turn_delta.x;
	camera->yaw = fmodf(game_state->camera.yaw, 2.0f * PI); 
	
	Vec3 move_direction = Vec3{0, 0, 0};
	if (game_input->controllers[0].up.ended_down) {
    	move_direction.x += -sin(camera->yaw);
    	move_direction.z += -cos(camera->yaw);
	}
	if (game_input->controllers[0].down.ended_down) {
    	move_direction.x += sin(camera->yaw);
    	move_direction.z += cos(camera->yaw);
	}
	if (game_input->controllers[0].left.ended_down) {
    	move_direction.x += -cos(camera->yaw);
    	move_direction.z +=  sin(camera->yaw);
	}
	if (game_input->controllers[0].right.ended_down) {
    	move_direction.x +=  cos(camera->yaw);
    	move_direction.z += -sin(camera->yaw);
	}
    move_direction = normalize(move_direction);

	camera->position += MOVE_SPEED * move_direction * frame_dt;

    //NOTE: Debugging
	if (button_was_released(game_input->controllers[0].draw_toggle)) {
    	should_draw = !should_draw;
	}
  
	// update animations
    /*if (player_anim->elapsed_seconds > player_anim->seconds_per_frame) {
        player_anim->elapsed_seconds = 0.0f;
        player_anim->current_frame.x += 1;
        player_anim->current_frame.x %= player_anim->num_frames.x;
    }
    else {
        player_anim->elapsed_seconds += frame_dt;
    }
    */

    for (Entity &entity : game_state->entities) {
        if (!entity.exists) continue;

        //update

        switch (entity.type) {
            case entity_type_player: {
                // calculate movement force
                f32 player_force_magnitude = 300.0f;
                Vec3 player_force = move_direction * player_force_magnitude;

                // calculate drag force.
                // Not physically accurate, but good enough for now
                f32 player_drag_coeff = 2.0f;
                Vec3 drag_force  = player_drag_coeff * 
                                   -Vec3{entity.dp.x, entity.dp.y, 0.0f};

                player_force += drag_force;
                
            	entity.dp += (player_force / entity.mass) * frame_dt;
            	entity.p += entity.dp * frame_dt;
            	//printf("player force: %.3f, %.3f, %.3f; vel: %.3f, %.3f, %.3f\n", player_force.x, player_force.y, player_force.z, entity.dp.x, entity.dp.y, entity.dp.z);
            	
                // collision. TODO: dont check against all other entities if entity count goes higher
            	for (Entity &col_entity : game_state->entities) {
                	if (col_entity.type == entity_type_wall) {
                    	bool intersects = AABB_intersects(
                        	Vec2{entity.p.x, entity.p.y}, entity.collide_rect_size,
                        	Vec2{col_entity.p.x, col_entity.p.y}, col_entity.collide_rect_size);
                	}
            	}
            } break;
            case entity_type_frog: {
                Entity *player = get_entity(game_state, 0);

                Vec3 force = Vec3{0.0f, 0.0f, 0.0f};
                if (length_sq(entity.dp) < 0.1f) {
                    // standing still.                     
                    if (distance(entity.p, player->p) < FROG_AVOIDANCE_THRESHOLD) {
                        force = 20000.0f * normalize(entity.p - player->p);

            	// printf("frog force: %.3f, %.3f, %.3f; vel: %.3f, %.3f, %.3f\n", force.x, force.y, force.z, entity.dp.x, entity.dp.y, entity.dp.z);
                    }
                }

                Vec3 drag_force = 1.9f * -Vec3{entity.dp.x, entity.dp.y, 0.0f};
                force += drag_force;
                entity.dp += force * frame_dt;
                entity.p  += entity.dp * frame_dt;

            } break;
        }
        
        /***** Begin drawing *****/
        switch (entity.type) {
            case entity_type_player: {
                // TODO: add in 4 directions with separate textures for up and down
                // NOTE: not sure if entity.dp or the force is more intuitive
                s32 direction = sign(entity.dp.x);
                if (direction == 0) direction = 1;

                //printf("dp: %f, dir: %d\n", entity.dp.x, direction);
                // TODO: visualize z coord as y coord change
                #if DEBUG_COLLISION
                draw_rect(Vec2{entity.p.x, entity.p.y}, entity.collide_rect_size, Vec3{0.0f, 0.5f, 0.1f});
                #endif
                /*draw_rect(game_state->player_texture,
                   Vec2{entity.p.x, entity.p.y} + direction * player_anim->center_offset,
                   direction, player_anim->current_frame,
                   player_anim->num_frames, 4.0f);
                */
            } break;
            
            case entity_type_wall: {
                //draw_rect(Vec2{entity.p.x, entity.p.y} , entity.collide_rect_size, entity.color);
            } break;

            case entity_type_frog: {
                s32 direction = sign(entity.dp.x); 
                if (direction == 0) direction = 1;
                
                TextureHandle current_tex = game_state->frog_sit_texture;
                if (length_sq(entity.dp) > 30.0f) 
                    current_tex = game_state->frog_jump_texture;
                
                //draw_rect(current_tex,
                //   Vec2{entity.p.x, entity.p.y},
                //   direction, Ivec2{0, 0}, Ivec2{1, 1}, 2.0f);
            } break;
        }

    }

    clear_color({0.3f, 0.4f, 0.5f, 1.0f});
    if (should_draw) {
        //draw_triangle(window_dims.x, window_dims.y, camera->position, camera->pitch, camera->yaw);
        draw_mesh(ground, window_dims.x, window_dims.y, camera->position, camera->pitch, camera->yaw);
    }
    //draw_rect(player->position, Vec2{20, 20});

    finish_frame(window_dims.x, window_dims.y);
    /***** End drawing *****/
}
