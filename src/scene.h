#ifndef SCENE_H
#define SCENE_H

#include "sphere.h"
#include "light.h"

#define MAX_SPHERES 10
#define MAX_LIGHTS 5
#define MAX_DEPTH 5

typedef struct {
    Sphere spheres[MAX_SPHERES];
    int sphere_count;
    Light lights[MAX_LIGHTS];
    int light_count;
    Vector3 background_color;
} Scene;

Scene scene_create();
void scene_add_sphere(Scene* scene, Sphere sphere);
void scene_add_light(Scene* scene, Light light);
Vector3 scene_trace(Scene* scene, Ray ray, int depth);
int scene_closest_hit(Scene* scene, Ray ray, double t_min, double t_max, Hit* hit);

#endif
