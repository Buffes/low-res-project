#pragma once
#include "root.h"
#include "renderer.h"

u32 vertex_array_object; // NOTE: multiple vaos can be used if faster
u32 vertex_buffer; // NOTE: More than one should probably exist later
u32 index_buffer;  // NOTE: More than one should probably exist later
u32 render_fb;
u32 depth_stencil_rb;
u32 color_rb;

// 16:9
// TODO: handle different resolutions and aspect ratios
s32 render_width  = 960 / 4;
s32 render_height = 540 / 4;

// Shaders

const char *vert_shader_str = R"str(
    #version 410 core

    layout (location = 0) in vec3 pos;

    uniform mat4 model_view_proj;

    out vec4 color;

    void main() {
        gl_Position = model_view_proj * vec4(pos, 1.0);
        color = vec4(0.0, 0.8, 0.4, 1.0);
    }
)str";

const char *frag_shader_str = R"str(
    #version 410 core 

    out vec4 frag_color;

    in vec4 color;
    
    void main() {
        frag_color = color;
    }
)str";

void setup_render_state() {
    // settings
    glEnable(GL_CULL_FACE);  
    
    // We use one single vao, and vertexAttribPointer calls when we draw. 
    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);
    
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

    // framebuffer for doing low-res rendering
    glGenFramebuffers(1, &render_fb);
    glBindFramebuffer(GL_FRAMEBUFFER, render_fb);

    glGenRenderbuffers(1, &depth_stencil_rb);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, render_width, render_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_stencil_rb);
        
    glGenRenderbuffers(1, &color_rb);
    glBindRenderbuffer(GL_RENDERBUFFER, color_rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, render_width, render_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_rb);
    glEnable(GL_DEPTH_TEST);

    // shaders
    u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vert_shader_str, NULL);
    glCompileShader(vertex_shader);
        
    u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &frag_shader_str, NULL);
    glCompileShader(fragment_shader); 

    // check compilation success
    int  success;
    char info[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info);
        printf("Vertex shader compilation failed:\n");
        printf("%s\n", info);
    }
    
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info);
        printf("Fragment shader compilation failed:\n");
        printf("%s\n", info);
    }

    u32 shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    
    glGetShaderiv(fragment_shader, GL_LINK_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader_program, 512, NULL, info);
        printf("Shader program linking failed:\n");
        printf("%s\n", info);
    }

    glUseProgram(shader_program);
}

Mat4 model_view_projection(
    f32 camera_pitch, 
    f32 camera_yaw, 
    Vec3 camera_pos, 
    f32 near_plane, 
    f32 far_plane,
    f32 aspect_ratio,
    f32 fov) { 
    Mat4 model_matrix = IDENTITY4;

    // view matrix from pitch and yaw.
    // NOTE: right handed coordinate system.
    //       Y is up, 0 pitch and yaw will look down negative Z axis.
    f32 cos_pitch = cos(camera_pitch);
    f32 sin_pitch = sin(camera_pitch);
    f32 cos_yaw   = cos(camera_yaw);
    f32 sin_yaw   = sin(camera_yaw);
    // R_yaw * R_pitch
    Vec3 x_axis = {cos_yaw, 0, -sin_yaw};
    Vec3 y_axis = {sin_yaw * sin_pitch, cos_pitch, cos_yaw * sin_pitch};
    Vec3 z_axis = {sin_yaw * cos_pitch, -sin_pitch, cos_pitch * cos_yaw};

    Mat4 view_matrix = {
        x_axis.x, y_axis.x, z_axis.x, 0,
        x_axis.y, y_axis.y, z_axis.y, 0,
        x_axis.z, y_axis.z, z_axis.z, 0,
        -dot(x_axis, camera_pos), -dot(y_axis, camera_pos), -dot(z_axis, camera_pos), 1
    };

    f32 n = near_plane; // near plane distance
    f32 f = far_plane; // far plane distance
    f32 focal_length = 1.0f / tan(fov / 2.0f);
    
    Mat4 projection_matrix {
        focal_length, 0, 0, 0,
        0, focal_length / aspect_ratio, 0, 0, 
        0, 0, (n + f) / (n - f), -1,
        0, 0, (2.0f * n * f) / (n - f), 0
    };

    return projection_matrix * view_matrix * model_matrix;
}


void draw_triangle(s32 screen_width, s32 screen_height, Vec3 camera_pos, f32 pitch, f32 yaw) {
    static f32 vertices[] = {
        0.0f,  6.0f, -1.0f,
       -6.0f, -4.0f, -1.0f,
        6.0f, -4.0f, -1.0f,
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    Mat4 mvp = model_view_projection(
        pitch,
        yaw, 
        camera_pos, 
        1.0f, 
        100.0f,
        (f32)screen_height / (f32)screen_width,
        PI_HALF); 

    glUniformMatrix4fv(0, 1, false, mvp.el);

    // draw to the low-res buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_fb);
    glViewport(0, 0, render_width, render_height);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}


void draw_mesh(Mesh mesh, s32 screen_width, s32 screen_height, Vec3 camera_pos, f32 pitch, f32 yaw) {
    Mat4 mvp = model_view_projection(
        pitch,
        yaw, 
        camera_pos, 
        1.0f, 
        100.0f,
        (f32)screen_height / (f32)screen_width,
        PI_HALF); 

    glUniformMatrix4fv(0, 1, false, mvp.el);

    glBufferData(GL_ARRAY_BUFFER, mesh.num_vertices * sizeof(Vec3), mesh.vertices, GL_STATIC_DRAW); 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.num_indices * sizeof(u32), mesh.indices, GL_STATIC_DRAW); 

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_fb);
    glViewport(0, 0, render_width, render_height);
    glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, (void*)0);
}


void draw_cube(s32 screen_width, s32 screen_height) {
    static f32 vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
    };

    //TODO: index array

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // draw to the low-res buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_fb);
    glViewport(0, 0, render_width, render_height);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void finish_frame(s32 screen_width, s32 screen_height) {
    // blit low-res buffer to the screen buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, render_width, render_height, // source
                      0, 0, screen_width, screen_height, // destination
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                      GL_NEAREST); // NOTE: not sure if the stencil buffer bit should be set here
}

void clear_color(Vec4 color) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_fb);
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
