#include "scene.h"
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Texture* scene_load_normal_map(Scene* scene, const char* filename) {
    return scene_load_texture(scene, filename, TEXTURE_TYPE_NORMAL);
}

void scene_free_textures(Scene* scene) {
    for (int i = 0; i < scene->texture_count; i++) {
        if (scene->textures[i].data) {
            stbi_image_free(scene->textures[i].data);
        }
    }
    scene->texture_count = 0;
    
    if (scene->environment_map && scene->environment_map->data) {
        stbi_image_free(scene->environment_map->data);
        scene->environment_map = NULL;
    }
}

Texture* scene_load_texture(Scene* scene, const char* filename, int type) {
    if (scene->texture_count >= MAX_TEXTURES) return NULL;
    
    Texture* tex = &scene->textures[scene->texture_count];
    tex->data = stbi_load(filename, &tex->width, &tex->height, &tex->channels, 3);
    tex->type = type;
    
    if (tex->data) {
        scene->texture_count++;
        return tex;
    }
    return NULL;
}

Texture* scene_load_environment_map(Scene* scene, const char* filename) {
    scene->environment_map = (Texture*)malloc(sizeof(Texture));
    if (!scene->environment_map) return NULL;
    
    scene->environment_map->data = stbi_load(filename, 
                                           &scene->environment_map->width,
                                           &scene->environment_map->height,
                                           &scene->environment_map->channels, 3);
    
    if (scene->environment_map->data) {
        scene->environment_map->type = TEXTURE_TYPE_COLOR;
        return scene->environment_map;
    }
    
    free(scene->environment_map);
    scene->environment_map = NULL;
    return NULL;
}

Vector3 sample_environment_map(Scene* scene, Vector3 direction) {
    if (!scene->environment_map || !scene->environment_map->data) {
        return scene->background_color;
    }
    
    // Convert direction vector to spherical coordinates
    double phi = atan2(direction.z, direction.x);
    double theta = acos(fmin(1.0, fmax(-1.0, direction.y)));
    
    // Convert to UV coordinates
    double u = (phi + M_PI) / (2.0 * M_PI);
    double v = theta / M_PI;
    
    // Calculate pixel coordinates
    int x = (int)(u * scene->environment_map->width) % scene->environment_map->width;
    int y = (int)(v * scene->environment_map->height) % scene->environment_map->height;
    
    // Ensure positive coordinates
    x = (x + scene->environment_map->width) % scene->environment_map->width;
    y = (y + scene->environment_map->height) % scene->environment_map->height;
    
    // Sample environment map
    int idx = (y * scene->environment_map->width + x) * scene->environment_map->channels;
    return vector_create(
        scene->environment_map->data[idx] / 255.0,
        scene->environment_map->data[idx + 1] / 255.0,
        scene->environment_map->data[idx + 2] / 255.0
    );
}

Scene scene_create() {
    Scene scene = {
        .sphere_count = 0,
        .light_count = 0,
        .mesh_count = 0,
        .aperture = 0.1,        // Default aperture size
        .focal_distance = 5.0,  // Default focal distance
        .background_color = {0.2, 0.2, 0.2},
        .animation_state = animation_state_create(30.0),  // Default 30 FPS
        .motion_blur_intensity = 0.5  // Default motion blur intensity
    };
    
    // Initialize animation tracks
    for (int i = 0; i < MAX_SPHERES; i++) {
        scene.sphere_animations[i] = NULL;
    }
    for (int i = 0; i < MAX_MESHES; i++) {
        scene.mesh_animations[i] = NULL;
    }
    for (int i = 0; i < MAX_LIGHTS; i++) {
        scene.light_animations[i] = NULL;
    }
    
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

void scene_add_mesh(Scene* scene, Mesh mesh) {
    if (scene->mesh_count < MAX_MESHES) {
        scene->meshes[scene->mesh_count++] = mesh;
    }
}

int scene_closest_hit(Scene* scene, Ray ray, double t_min, double t_max, Hit* hit) {
    Hit temp_hit;
    int hit_anything = 0;
    double closest_so_far = t_max;

    // Check sphere intersections with animation support
    for (int i = 0; i < scene->sphere_count; i++) {
        Sphere current_sphere = scene->spheres[i];
        
        // Apply animation if exists
        if (scene->sphere_animations[i]) {
            Keyframe current_state = animation_track_interpolate(
                scene->sphere_animations[i],
                ray.time
            );
            
            // Update sphere transform
            current_sphere.center = current_state.position;
            // Store velocity for motion blur calculations
            Vector3 velocity = current_state.velocity;
            
            // Apply velocity-based motion blur
            if (scene->motion_blur_intensity > 0.0) {
                current_sphere.center = vector_add(
                    current_sphere.center,
                    vector_multiply(velocity, ray.time - scene->animation_state.current_time)
                );
            }
        }
        
        if (sphere_intersect(&current_sphere, ray, t_min, closest_so_far, &temp_hit)) {
            hit_anything = 1;
            closest_so_far = temp_hit.t;
            *hit = temp_hit;
        }
    }

    // Check mesh intersections with animation support
    for (int i = 0; i < scene->mesh_count; i++) {
        Mesh current_mesh = scene->meshes[i];
        
        // Apply animation if exists
        if (scene->mesh_animations[i]) {
            Keyframe current_state = animation_track_interpolate(
                scene->mesh_animations[i],
                ray.time
            );
            
            // Update mesh transform
            current_mesh.position = current_state.position;
            current_mesh.rotation = current_state.rotation;
            current_mesh.scale = current_state.scale;
        }
        
        if (mesh_intersect(&current_mesh, ray, t_min, closest_so_far, &temp_hit)) {
            hit_anything = 1;
            closest_so_far = temp_hit.t;
            temp_hit.mesh = &current_mesh;
            *hit = temp_hit;
        }
    }

    return hit_anything;
}

static Ray generate_defocus_ray(Scene* scene, Ray original_ray, Vector3 focal_point) {
    // Generate random point in aperture disk
    double r = scene->aperture * sqrt((double)rand() / RAND_MAX);
    double theta = 2.0 * M_PI * ((double)rand() / RAND_MAX);
    
    Vector3 offset = vector_create(
        r * cos(theta),
        r * sin(theta),
        0.0
    );
    
    Vector3 origin = vector_add(original_ray.origin, offset);
    Vector3 direction = vector_normalize(vector_subtract(focal_point, origin));
    
    return ray_create(origin, direction);
}

static Vector3 trace_chromatic(Scene* scene, Ray ray, int depth, double wavelength_offset) {
    Hit hit;
    if (depth <= 0) return vector_create(0, 0, 0);
    
    // Adjust IOR based on wavelength for chromatic aberration
    if (wavelength_offset != 0.0) {
        ray.wavelength_offset = wavelength_offset;
    }

    if (scene_closest_hit(scene, ray, 0.001, DBL_MAX, &hit)) {
        Vector3 color = vector_create(0, 0, 0);
        
        // Adjust IOR for chromatic aberration
        if (hit.sphere) {
            double wavelength_ior = hit.sphere->fresnel_ior + 
                (wavelength_offset * hit.sphere->dispersion);
            
            // Calculate refraction
            Vector3 view_dir = vector_normalize(vector_multiply(ray.direction, -1.0));
            double cos_theta = vector_dot(view_dir, hit.normal);
            double ior_ratio = cos_theta > 0 ? 1.0 / wavelength_ior : wavelength_ior;
            
            Vector3 refracted = vector_multiply(ray.direction, ior_ratio);
            Ray refract_ray = ray_create(hit.point, refracted);
            color = scene_trace(scene, refract_ray, depth - 1);
        }
        
        return color;
    }
    
    return scene->background_color;
}

Vector3 scene_trace(Scene* scene, Ray ray, int depth) {
    Hit hit;
    if (depth <= 0) return vector_create(0, 0, 0);
    
    // For transparent objects, trace different wavelengths
    Vector3 color = vector_create(0, 0, 0);
    if (depth == MAX_DEPTH) {  // Only do chromatic aberration on primary rays
        color.x = trace_chromatic(scene, ray, depth, 0.02).x;  // Red wavelength
        color.y = trace_chromatic(scene, ray, depth, 0.0).y;   // Green wavelength
        color.z = trace_chromatic(scene, ray, depth, -0.02).z; // Blue wavelength
        return color;
    }

    // Calculate focal point for depth of field
    Vector3 focal_point = ray_point_at(ray, scene->focal_distance);
    
    // If aperture is significant, use depth of field
    if (scene->aperture > 0.001) {
        ray = generate_defocus_ray(scene, ray, focal_point);
    }

    if (scene_closest_hit(scene, ray, 0.001, DBL_MAX, &hit)) {
        Vector3 color = vector_create(0, 0, 0);
        
        // Calculate lighting with animated lights
        for (int i = 0; i < scene->light_count; i++) {
            Light current_light = scene->lights[i];
            
            // Apply animation if exists
            if (scene->light_animations[i]) {
                Keyframe current_state = animation_track_interpolate(
                    scene->light_animations[i],
                    ray.time
                );
                
                // Update light position
                current_light.position = current_state.position;
            }
            
            const int shadow_samples = 8;   // Reduced samples for better performance
            Vector3 light_contribution = vector_create(0, 0, 0);
            
            for (int sample = 0; sample < shadow_samples; sample++) {
                Vector3 light_pos = light_random_position(current_light);
                Vector3 light_dir = vector_normalize(vector_subtract(light_pos, hit.point));
                
                // Shadow ray
                Ray shadow_ray = ray_create(hit.point, light_dir);
                Hit shadow_hit;
                double light_distance = vector_length(vector_subtract(light_pos, hit.point));
                
                if (!scene_closest_hit(scene, shadow_ray, 0.001, light_distance, &shadow_hit)) {
                    // Calculate diffuse component with surface normal
                    double diff = fmax(0.0, vector_dot(hit.normal, light_dir));
                    
                    // Get texture color if available
                    Vector3 surface_color = hit.sphere->color;
                    if (hit.sphere->color_texture) {
                        Vector2 tex_coord = calculate_sphere_uv(hit.point, hit.sphere->center, hit.sphere->texture_scale);
                        surface_color = vector_multiply_vec(surface_color, sample_texture(tex_coord, hit.sphere->color_texture));
                    }
                    
                    // Calculate specular component with glossiness
                    Vector3 view_dir = vector_normalize(vector_multiply(ray.direction, -1));
                    Vector3 reflect_dir = vector_reflect(vector_multiply(light_dir, -1), hit.normal);
                    double gloss_power = 2.0 + hit.sphere->glossiness * 126.0;
                    double spec = pow(fmax(vector_dot(view_dir, reflect_dir), 0.0), gloss_power);
                    
                    // Combine diffuse and specular components
                    Vector3 diffuse = vector_multiply_vec(surface_color, current_light.color);
                    Vector3 specular = vector_multiply(current_light.color, hit.sphere->glossiness * spec);
                    Vector3 sample_contribution = vector_multiply(
                        vector_add(diffuse, specular),
                        diff * current_light.intensity
                    );
                    light_contribution = vector_add(light_contribution, sample_contribution);
                }
            }
            
            // Average the light contributions
            light_contribution = vector_divide(light_contribution, shadow_samples);
            color = vector_add(color, light_contribution);
        }

        // Calculate Fresnel reflection using Schlick's approximation
        if (depth > 0 && hit.sphere) {
            Vector3 view_dir = vector_normalize(vector_multiply(ray.direction, -1.0));
            double cos_theta = fabs(vector_dot(view_dir, hit.normal));
            
            // Calculate physically accurate Fresnel term
            double r0 = (hit.sphere->fresnel_ior - 1.0) / (hit.sphere->fresnel_ior + 1.0);
            r0 = r0 * r0;
            
            // Compute roughness-adjusted Fresnel factor
            double roughness_factor = hit.sphere->roughness * hit.sphere->roughness;
            double fresnel_factor = r0 + (1.0 - r0) * pow(1.0 - cos_theta, 5.0) * hit.sphere->fresnel_power;
            
            // Adjust for metallic surfaces
            if (hit.sphere->metallic > 0.0) {
                fresnel_factor = fresnel_factor * (1.0 - roughness_factor) + hit.sphere->metallic * roughness_factor;
            }
            
            // Blend with material's base reflectivity
            double final_reflectivity = hit.sphere->reflectivity * fresnel_factor;
            
            if (final_reflectivity > 0.0) {
                Vector3 reflected = vector_reflect(ray.direction, hit.normal);
                Ray reflect_ray = ray_create(hit.point, reflected);
                reflect_ray.time = ray.time;  // Maintain time consistency for animations
                Vector3 reflect_color = scene_trace(scene, reflect_ray, depth - 1);
                color = vector_add(color, vector_multiply(reflect_color, final_reflectivity));
            }
        }

        return color;
    }

    return scene->background_color;
}
