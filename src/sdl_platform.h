#ifndef sdl_platform_h
#define sdl_platform_h

struct TextureHandle {
    SDL_Texture *sdl_texture;
    u32 width;
    u32 height;
};

TextureHandle load_texture(char *filename);
void draw_rect(Vec2 center, Vec2 size, Vec3 color = Vec3{1, 1, 1});

void draw_rect(TextureHandle texture, Vec2 center, s32 direction, Ivec2 anim_frame, Ivec2 anim_num_frames, s32 scale = 1, Vec3 color = Vec3{1, 1, 1});

#endif // sdl_platform_h_
