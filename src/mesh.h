#ifndef MESH_H
#define MESH_H

#include "common.h"
#include "ray.h"

#define MAX_TRIANGLES 1000

typedef struct {
    Vector3 vertices[3];     // Three vertices defining the triangle
    Vector3 normals[3];      // Per-vertex normals for smooth shading
    Vector3 face_normal;     // Face normal (precomputed)
    int smooth_shading;      // Flag for smooth shading
} Triangle;

#define MAX_VERTICES 2000

// Mesh structure definition
typedef struct Mesh {
    Triangle triangles[MAX_TRIANGLES];
    int triangle_count;
    Vector3* vertices;         // Dynamic vertex array
    int* vertex_indices;       // Triangle vertex indices
    int vertex_count;          // Total number of vertices
    Vector3 position;          // Mesh position in world space
    Vector3 rotation;          // Mesh rotation (euler angles)
    Vector3 scale;            // Mesh scale
    Vector3 color;            // Mesh color
    double reflectivity;      // Mesh reflectivity
    double fresnel_ior;       // Index of Refraction for Fresnel calculations
    double fresnel_power;     // Controls strength of Fresnel effect
    Texture* normal_map;      // Normal map for enhanced surface detail
    int use_smooth_shading;   // Global smooth shading flag
} Mesh;

// Function declarations
void mesh_compute_smooth_normals(Mesh* mesh);
void mesh_set_smooth_shading(Mesh* mesh, int enable);
Mesh mesh_create(Vector3 position, Vector3 rotation, Vector3 scale, Vector3 color, double reflectivity);
void mesh_add_triangle(Mesh* mesh, Vector3 v1, Vector3 v2, Vector3 v3);
int mesh_intersect(Mesh* mesh, Ray ray, double t_min, double t_max, Hit* hit);
int ray_triangle_intersect(Ray ray, Triangle triangle, double t_min, double t_max, Hit* hit);

// Utility functions
void mesh_compute_triangle_normal(Triangle* triangle);
Mesh create_cube_mesh(Vector3 position, double size, Vector3 color, double reflectivity);
Vector3 calculate_mesh_normal(Vector3 normal, Vector2Double tex_coord, Texture* normal_map);

#endif
