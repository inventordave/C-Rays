#include "mesh.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Matrix operations
typedef struct {
    double m[4][4];
} Matrix4x4;

Matrix4x4 matrix_identity() {
    Matrix4x4 m = {{{0}}};
    for (int i = 0; i < 4; i++) {
        m.m[i][i] = 1.0;
    }
    return m;
}

Matrix4x4 matrix_multiply(Matrix4x4 a, Matrix4x4 b) {
    Matrix4x4 result = {{{0}}};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                result.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return result;
}

Matrix4x4 create_rotation_matrix(Vector3 rotation) {
    Matrix4x4 rx = matrix_identity();
    Matrix4x4 ry = matrix_identity();
    Matrix4x4 rz = matrix_identity();
    
    // Rotation around X axis
    double cx = cos(rotation.x);
    double sx = sin(rotation.x);
    rx.m[1][1] = cx;
    rx.m[1][2] = -sx;
    rx.m[2][1] = sx;
    rx.m[2][2] = cx;
    
    // Rotation around Y axis
    double cy = cos(rotation.y);
    double sy = sin(rotation.y);
    ry.m[0][0] = cy;
    ry.m[0][2] = sy;
    ry.m[2][0] = -sy;
    ry.m[2][2] = cy;
    
    // Rotation around Z axis
    double cz = cos(rotation.z);
    double sz = sin(rotation.z);
    rz.m[0][0] = cz;
    rz.m[0][1] = -sz;
    rz.m[1][0] = sz;
    rz.m[1][1] = cz;
    
    // Combine rotations: Z * Y * X
    return matrix_multiply(rz, matrix_multiply(ry, rx));
}

Matrix4x4 create_transform_matrix(Vector3 position, Vector3 rotation, Vector3 scale) {
    Matrix4x4 transform = matrix_identity();
    Matrix4x4 rot = create_rotation_matrix(rotation);
    
    // Apply scale
    for (int i = 0; i < 3; i++) {
        rot.m[i][0] *= scale.x;
        rot.m[i][1] *= scale.y;
        rot.m[i][2] *= scale.z;
    }
    
    // Copy rotation and scale
    memcpy(transform.m, rot.m, sizeof(rot.m));
    
    // Set translation
    transform.m[0][3] = position.x;
    transform.m[1][3] = position.y;
    transform.m[2][3] = position.z;
    
    return transform;
}

Vector3 transform_point(Matrix4x4 matrix, Vector3 point) {
    double x = matrix.m[0][0] * point.x + matrix.m[0][1] * point.y + matrix.m[0][2] * point.z + matrix.m[0][3];
    double y = matrix.m[1][0] * point.x + matrix.m[1][1] * point.y + matrix.m[1][2] * point.z + matrix.m[1][3];
    double z = matrix.m[2][0] * point.x + matrix.m[2][1] * point.y + matrix.m[2][2] * point.z + matrix.m[2][3];
    return vector_create(x, y, z);
}

Vector3 transform_vector(Matrix4x4 matrix, Vector3 vector) {
    double x = matrix.m[0][0] * vector.x + matrix.m[0][1] * vector.y + matrix.m[0][2] * vector.z;
    double y = matrix.m[1][0] * vector.x + matrix.m[1][1] * vector.y + matrix.m[1][2] * vector.z;
    double z = matrix.m[2][0] * vector.x + matrix.m[2][1] * vector.y + matrix.m[2][2] * vector.z;
    return vector_create(x, y, z);
}


Mesh mesh_create(Vector3 position, Vector3 rotation, Vector3 scale, Vector3 color, double reflectivity) {
    Mesh mesh = {
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .color = color,
        .reflectivity = reflectivity,
        .triangle_count = 0,
        .vertex_count = 0,
        .vertices = NULL,
        .vertex_indices = NULL,
        .use_smooth_shading = 0
    };
    
    // Allocate initial arrays
    mesh.vertices = (Vector3*)malloc(MAX_VERTICES * sizeof(Vector3));
    mesh.vertex_indices = (int*)malloc(MAX_TRIANGLES * 3 * sizeof(int));
    
    if (!mesh.vertices || !mesh.vertex_indices) {
        fprintf(stderr, "Failed to allocate mesh buffers\n");
        if (mesh.vertices) free(mesh.vertices);
        if (mesh.vertex_indices) free(mesh.vertex_indices);
        mesh.vertices = NULL;
        mesh.vertex_indices = NULL;
    }
    
    return mesh;
}

void mesh_set_smooth_shading(Mesh* mesh, int enable) {
    mesh->use_smooth_shading = enable;
    for (int i = 0; i < mesh->triangle_count; i++) {
        mesh->triangles[i].smooth_shading = enable;
    }
    if (enable) {
        mesh_compute_smooth_normals(mesh);
    }
}

void mesh_compute_triangle_normal(Triangle* triangle) {
    Vector3 edge1 = vector_subtract(triangle->vertices[1], triangle->vertices[0]);
    Vector3 edge2 = vector_subtract(triangle->vertices[2], triangle->vertices[0]);
    triangle->face_normal = vector_normalize(vector_cross(edge1, edge2));
    
    // Initialize vertex normals with face normal if smooth shading is not enabled
    if (!triangle->smooth_shading) {
        for (int i = 0; i < 3; i++) {
            triangle->normals[i] = triangle->face_normal;
        }
    }
}

// Function to compute smooth vertex normals for a mesh
void mesh_compute_smooth_normals(Mesh* mesh) {
    // Initialize vertex normal accumulation arrays
    Vector3* vertex_normals = (Vector3*)calloc(mesh->vertex_count, sizeof(Vector3));
    double* vertex_weights = (double*)calloc(mesh->vertex_count, sizeof(double));
    
    // Accumulate weighted face normals for each vertex
    for (int i = 0; i < mesh->triangle_count; i++) {
        Triangle* tri = &mesh->triangles[i];
        if (!tri->smooth_shading) continue;
        
        // Calculate edges
        Vector3 edges[3] = {
            vector_subtract(tri->vertices[1], tri->vertices[0]),
            vector_subtract(tri->vertices[2], tri->vertices[1]),
            vector_subtract(tri->vertices[0], tri->vertices[2])
        };
        
        // Calculate angles at each vertex
        double angles[3];
        for (int j = 0; j < 3; j++) {
            Vector3 e1 = vector_normalize(vector_multiply(edges[j], -1));
            Vector3 e2 = vector_normalize(edges[(j + 2) % 3]);
            angles[j] = acos(fmax(-1.0, fmin(1.0, vector_dot(e1, e2))));
        }
        
        // Accumulate weighted normals
        for (int j = 0; j < 3; j++) {
            int vertex_index = mesh->vertex_indices[i * 3 + j];
            Vector3 weighted_normal = vector_multiply(tri->face_normal, angles[j]);
            vertex_normals[vertex_index] = vector_add(vertex_normals[vertex_index], weighted_normal);
            vertex_weights[vertex_index] += angles[j];
        }
    }
    
    // Normalize accumulated weighted normals
    for (int i = 0; i < mesh->vertex_count; i++) {
        if (vertex_weights[i] > 0.0) {
            vertex_normals[i] = vector_normalize(vertex_normals[i]);
        }
    }
    
    // Assign smooth normals to triangles
    for (int i = 0; i < mesh->triangle_count; i++) {
        Triangle* tri = &mesh->triangles[i];
        if (tri->smooth_shading) {
            for (int j = 0; j < 3; j++) {
                int vertex_index = mesh->vertex_indices[i * 3 + j];
                tri->normals[j] = vertex_normals[vertex_index];
            }
        }
    }
    
    free(vertex_normals);
    free(vertex_weights);
}

void mesh_add_triangle(Mesh* mesh, Vector3 v1, Vector3 v2, Vector3 v3) {
    if (mesh->triangle_count < MAX_TRIANGLES) {
        Triangle triangle;
        triangle.vertices[0] = v1;
        triangle.vertices[1] = v2;
        triangle.vertices[2] = v3;
        mesh_compute_triangle_normal(&triangle);
        mesh->triangles[mesh->triangle_count++] = triangle;
    }
}

int ray_triangle_intersect(Ray ray, Triangle triangle, double t_min, double t_max, Hit* hit) {
    Vector3 edge1 = vector_subtract(triangle.vertices[1], triangle.vertices[0]);
    Vector3 edge2 = vector_subtract(triangle.vertices[2], triangle.vertices[0]);
    
    // Calculate determinant
    Vector3 pvec = vector_cross(ray.direction, edge2);
    double det = vector_dot(edge1, pvec);
    
    // Two-sided triangle test
    if (fabs(det) < 0.000001) return 0;
    
    double inv_det = 1.0 / det;
    
    // Calculate barycentric coordinates
    Vector3 tvec = vector_subtract(ray.origin, triangle.vertices[0]);
    double u = vector_dot(tvec, pvec) * inv_det;
    if (u < 0.0 || u > 1.0) return 0;
    
    Vector3 qvec = vector_cross(tvec, edge1);
    double v = vector_dot(ray.direction, qvec) * inv_det;
    if (v < 0.0 || u + v > 1.0) return 0;
    
    double t = vector_dot(edge2, qvec) * inv_det;
    if (t < t_min || t > t_max) return 0;
    
    // Calculate intersection point
    hit->t = t;
    hit->point = ray_point_at(ray, t);
    
    // Calculate barycentric coordinates
    double w = 1.0 - u - v;
    
    if (triangle.smooth_shading) {
        // Enhanced smooth normal interpolation with proper weighting
        Vector3 interpolated_normal;
        
        // Calculate edge lengths for proper weighting
        double len01 = vector_length(edge1);
        double len02 = vector_length(edge2);
        double len12 = vector_length(vector_subtract(triangle.vertices[2], triangle.vertices[1]));
        
        // Calculate weights based on edge lengths and barycentric coordinates
        double weight0 = w * (len01 + len02);
        double weight1 = u * (len01 + len12);
        double weight2 = v * (len02 + len12);
        double total_weight = weight0 + weight1 + weight2;
        
        if (total_weight > 0.0) {
            weight0 /= total_weight;
            weight1 /= total_weight;
            weight2 /= total_weight;
            
            interpolated_normal = vector_add(
                vector_add(
                    vector_multiply(triangle.normals[0], weight0),
                    vector_multiply(triangle.normals[1], weight1)
                ),
                vector_multiply(triangle.normals[2], weight2)
            );
        } else {
            interpolated_normal = triangle.face_normal;
        }
        
        hit->normal = vector_normalize(interpolated_normal);
    } else {
        hit->normal = triangle.face_normal;
    }
    
    // Store barycentric coordinates for texture mapping
    hit->tex_coord.u = u;
    hit->tex_coord.v = v;
    
    return 1;
}

int mesh_intersect(Mesh* mesh, Ray ray, double t_min, double t_max, Hit* hit) {
    int hit_anything = 0;
    double closest_so_far = t_max;
    Hit temp_hit;
    
    // Create transformation matrix
    Matrix4x4 transform = create_transform_matrix(mesh->position, mesh->rotation, mesh->scale);
    Matrix4x4 inverse_transform = matrix_identity();  // TODO: Implement proper matrix inversion
    
    // Transform ray to mesh space
    Ray transformed_ray = ray;
    transformed_ray.origin = vector_subtract(ray.origin, mesh->position);  // Simple translation for now
    transformed_ray.origin = transform_point(inverse_transform, ray.origin);
    transformed_ray.direction = transform_vector(inverse_transform, ray.direction);
    transformed_ray.direction = vector_normalize(transformed_ray.direction);
    
    for (int i = 0; i < mesh->triangle_count; i++) {
        if (ray_triangle_intersect(transformed_ray, mesh->triangles[i], t_min, closest_so_far, &temp_hit)) {
            hit_anything = 1;
            closest_so_far = temp_hit.t;
            
            // Transform intersection point and normal back to world space
            temp_hit.point = transform_point(transform, temp_hit.point);
            temp_hit.normal = transform_vector(transform, temp_hit.normal);
            temp_hit.normal = vector_normalize(temp_hit.normal);
            
            *hit = temp_hit;
        }
    }
    
    return hit_anything;
}

Mesh create_cube_mesh(Vector3 position, double size, Vector3 color, double reflectivity) {
    Mesh mesh = mesh_create(position, vector_create(0, 0, 0), vector_create(1, 1, 1), color, reflectivity);
    
    double s = size / 2.0;
    Vector3 vertices[8] = {
        vector_create(-s, -s, -s), // 0: left bottom back
        vector_create(s, -s, -s),  // 1: right bottom back
        vector_create(s, s, -s),   // 2: right top back
        vector_create(-s, s, -s),  // 3: left top back
        vector_create(-s, -s, s),  // 4: left bottom front
        vector_create(s, -s, s),   // 5: right bottom front
        vector_create(s, s, s),    // 6: right top front
        vector_create(-s, s, s)    // 7: left top front
    };
    
    // Front face
    mesh_add_triangle(&mesh, vertices[4], vertices[5], vertices[6]);
    mesh_add_triangle(&mesh, vertices[4], vertices[6], vertices[7]);
    
    // Back face
    mesh_add_triangle(&mesh, vertices[1], vertices[0], vertices[2]);
    mesh_add_triangle(&mesh, vertices[2], vertices[0], vertices[3]);
    
    // Right face
    mesh_add_triangle(&mesh, vertices[5], vertices[1], vertices[6]);
    mesh_add_triangle(&mesh, vertices[6], vertices[1], vertices[2]);
    
    // Left face
    mesh_add_triangle(&mesh, vertices[0], vertices[4], vertices[3]);
    mesh_add_triangle(&mesh, vertices[3], vertices[4], vertices[7]);
    
    // Top face
    mesh_add_triangle(&mesh, vertices[3], vertices[7], vertices[2]);
    mesh_add_triangle(&mesh, vertices[2], vertices[7], vertices[6]);
    
    // Bottom face
    mesh_add_triangle(&mesh, vertices[4], vertices[0], vertices[5]);
    mesh_add_triangle(&mesh, vertices[5], vertices[0], vertices[1]);
    
    return mesh;
}
