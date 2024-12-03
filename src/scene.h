#ifndef SCENE_H
#define SCENE_H

#include "common.h"
#include "ray.h"
#include "sphere.h"
#include "light.h"
#include "mesh.h"
#include "animation.h"

#define MAX_SPHERES 10
#define MAX_LIGHTS 5
#define MAX_MESHES 10
#define MAX_DEPTH 5
#define MAX_NORMAL_MAPS 10

// Scene structure definition
typedef struct Scene {
    double aperture;       // Camera aperture size
    double focal_distance; // Distance to focal plane
    struct Sphere spheres[MAX_SPHERES];
    int sphere_count;
    Light lights[MAX_LIGHTS];
    int light_count;
    struct Mesh meshes[MAX_MESHES];
    int mesh_count;
    #define MAX_TEXTURES 20
    Texture textures[MAX_TEXTURES];
    int texture_count;
    Texture* environment_map;
    Vector3 background_color;
    
    // Animation support
    AnimationState animation_state;
    AnimationTrack* sphere_animations[MAX_SPHERES];
    AnimationTrack* mesh_animations[MAX_MESHES];
    AnimationTrack* light_animations[MAX_LIGHTS];
    double motion_blur_intensity;  // Controls strength of motion blur effect
} Scene;

// Function declarations
Scene scene_create(void);
void scene_add_sphere(Scene* scene, struct Sphere sphere);
void scene_add_light(Scene* scene, Light light);
void scene_add_mesh(Scene* scene, struct Mesh mesh);
Vector3 scene_trace(Scene* scene, Ray ray, int depth);
int scene_closest_hit(Scene* scene, Ray ray, double t_min, double t_max, Hit* hit);
Texture* scene_load_texture(Scene* scene, const char* filename, int type);
Texture* scene_load_environment_map(Scene* scene, const char* filename);
void scene_free_textures(Scene* scene);
Vector3 sample_environment_map(Scene* scene, Vector3 direction);

#endif