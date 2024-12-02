#ifndef APLIB_H
#define APLIB_H

#include "mesh.h"
#include "vector.h"
#include <stdio.h>

// APLIB mesh loading and manipulation functions
typedef struct {
    char magic[4];      // File format identifier
    int version;        // Format version
    int vertex_count;   // Number of vertices
    int triangle_count; // Number of triangles
} APLIBHeader;

// Arbitrary-precision vector functions
Vector3 aplib_vector_create(const char* x, const char* y, const char* z);
Vector3 aplib_vector_add(Vector3 a, Vector3 b);
Vector3 aplib_vector_subtract(Vector3 a, Vector3 b);
Vector3 aplib_vector_multiply(Vector3 v, const char* scalar);
Vector3 aplib_vector_divide(Vector3 v, const char* scalar);
double aplib_vector_dot(Vector3 a, Vector3 b);
Vector3 aplib_vector_cross(Vector3 a, Vector3 b);
double aplib_vector_length(Vector3 v);
Vector3 aplib_vector_normalize(Vector3 v);
Vector3 aplib_vector_reflect(Vector3 v, Vector3 normal);

// Original mesh functions
int aplib_load_mesh(const char* filename, Mesh* mesh);
int aplib_save_mesh(const char* filename, Mesh* mesh);

// Utility functions
void aplib_transform_mesh(Mesh* mesh, Vector3 position, Vector3 rotation, Vector3 scale);
void aplib_compute_normals(Mesh* mesh);

#endif
