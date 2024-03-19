#ifndef memory_h
#define memory_h

#define KILOBYTES(value) (         (value)*1024LL)
#define MEGABYTES(value) (KILOBYTES(value)*1024LL)
#define GIGABYTES(value) (MEGABYTES(value)*1024LL)

struct MemoryArena {
    size_t size;
    size_t used;
    u8 *start; 
};

#endif // memory_h
