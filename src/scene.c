#include "scene.h"
#include <float.h>
#include <math.h>

Scene scene_create() {
    Scene scene = {
        .sphere_count = 0,
        .light_count = 0,
        .background_color = {0.2, 0.2, 0.2}
    };
    return scene;
}

void scene_add_sphere(Scene* scene, Sphere sphere) {
    if (scene->sphere_count < MAX_SPHERES) {
        scene->spheres[scene->sphere_count++] = sphere;
    }
}

void scene_add_light(Scene* scene, Light light) {
    if (scene->light_count < MAX_LIGHTS) {
        scene->lights[scene->light_count++] = light;
    }
}

int scene_closest_hit(Scene* scene, Ray ray, double t_min, double t_max, Hit* hit) {
    Hit temp_hit;
    int hit_anything = 0;
    double closest_so_far = t_max;

    for (int i = 0; i < scene->sphere_count; i++) {
        if (sphere_intersect(&scene->spheres[i], ray, t_min, closest_so_far, &temp_hit)) {
            hit_anything = 1;
            closest_so_far = temp_hit.t;
            *hit = temp_hit;
        }
    }

    return hit_anything;
}

Vector3 scene_trace(Scene* scene, Ray ray, int depth) {
    Hit hit;
    if (depth <= 0) return vector_create(0, 0, 0);

    if (scene_closest_hit(scene, ray, 0.001, DBL_MAX, &hit)) {
        Vector3 color = vector_create(0, 0, 0);
        
        // Calculate lighting
        for (int i = 0; i < scene->light_count; i++) {
            Light light = scene->lights[i];
            Vector3 light_dir = vector_normalize(vector_subtract(light.position, hit.point));
            
            // Shadow ray
            Ray shadow_ray = ray_create(hit.point, light_dir);
            Hit shadow_hit;
            double light_distance = vector_length(vector_subtract(light.position, hit.point));
            
            if (!scene_closest_hit(scene, shadow_ray, 0.001, light_distance, &shadow_hit)) {
                double diff = fmax(0.0, vector_dot(hit.normal, light_dir));
                color = vector_add(color, vector_multiply(vector_multiply_vec(hit.sphere->color, light.color), diff * light.intensity));
            }
        }

        // Reflection
        if (hit.sphere->reflectivity > 0 && depth > 0) {
            Vector3 reflected = vector_reflect(ray.direction, hit.normal);
            Ray reflect_ray = ray_create(hit.point, reflected);
            Vector3 reflect_color = scene_trace(scene, reflect_ray, depth - 1);
            color = vector_add(color, vector_multiply(reflect_color, hit.sphere->reflectivity));
        }

        return color;
    }

    return scene->background_color;
}
