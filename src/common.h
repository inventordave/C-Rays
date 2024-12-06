#ifndef COMMON_H
#define COMMON_H

#include "vector.h"

// Texture structure definition
typedef struct {
    unsigned char* data;
    int width;
    int height;
    int channels;
    int type;  // 0 for color texture, 1 for normal map
} Texture;

// Define texture types
#define TEXTURE_TYPE_COLOR 0
#define TEXTURE_TYPE_NORMAL 1

// Forward declarations
struct Sphere;
struct Mesh;
struct Scene;

// Hit structure definition
typedef struct Hit {
    double t;
    Vector3 point;
    Vector3 normal;
    Vector2Double tex_coord;    // Texture coordinates (u,v)
    union {
        struct Sphere* sphere;
        struct Mesh* mesh;
    };
    int is_mesh;  // 0 for sphere, 1 for mesh
} Hit;

#endif
