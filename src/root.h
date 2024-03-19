/*
 * This file is the root for the unity build of this project.
 * The build passes sdl_platform.cpp to the compiler, so this
 * could just be put at the top of sdl_platform.cpp, but
 * it is separated into its own file to make the clangd lsp
 * work well, using the method from here:
 * https://www.frogtoss.com/labs/clangd-with-unity-builds.html
 *
 * To make the build work correctly:
 * #include this file in all .cpp files, and #include all .cpp files
 * and third party libs in here.
 */
 
#pragma once
#include <stdio.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#ifdef METAL
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#endif

#include <SDL2/SDL.h>

#ifdef OPENGL
#include "include/glad/src/glad.c"
#include <SDL2/SDL_opengl.h>
#endif

#define global static
#define persistent static
#define internal static

#if DEBUG
#define ASSERT(statement) \
    if (!(statement)) { \
        printf("Assertion failed.\n");\
        __builtin_trap(); \
    }
#else
#define ASSERT(statement)
#endif

#define ARRAY_COUNT(array) (sizeof(array)/sizeof((array)[0]))

#include "types.h"
#include "../../libs/el_math.h"
#include "../../libs/el_random.h"
#include "sdl_platform.h"
#include "memory.cpp"
#ifdef METAL
#include "renderer_metal.cpp"
#endif 
#ifdef OPENGL
#include "renderer_opengl.cpp"
#endif 
#include "game.cpp"
