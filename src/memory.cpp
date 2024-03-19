#pragma once
#include "root.h"
#include "memory.h"

#define reserve_struct(arena, type) (type*)_reserve_bytes(arena, sizeof(type))
#define reserve_array(arena, type, count) (type*)_reserve_bytes(arena, sizeof(type) * (count))

void *_reserve_bytes(MemoryArena *arena, size_t num_bytes) {
    //NOTE: presumes a zeroed memory region.
    ASSERT(arena->used + num_bytes <= arena->size);
    void *memory_region_start = arena->start + arena->used; 
    arena->used += num_bytes;
  
    return memory_region_start;
}

