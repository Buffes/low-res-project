#ifndef renderer_h_INCLUDED
#define renderer_h_INCLUDED

struct Mesh {
    u32 num_vertices;
    u32 num_indices;
    Vec3 *vertices;
    u32  *indices;
};

#endif // renderer_h_INCLUDED
