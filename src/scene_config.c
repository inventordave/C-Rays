#include "scene_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Vector3 parse_vector3(json_object* vec_obj) {
    Vector3 vec = {0};
    if (!json_object_is_type(vec_obj, json_type_object)) return vec;
    
    json_object* x_obj, *y_obj, *z_obj;
    json_object_object_get_ex(vec_obj, "x", &x_obj);
    json_object_object_get_ex(vec_obj, "y", &y_obj);
    json_object_object_get_ex(vec_obj, "z", &z_obj);
    
    if (x_obj && y_obj && z_obj) {
        vec.x = json_object_get_double(x_obj);
        vec.y = json_object_get_double(y_obj);
        vec.z = json_object_get_double(z_obj);
    }
    return vec;
}

static void load_texture_config(json_object* tex_obj, Texture** texture, Scene* scene) {
    if (!json_object_is_type(tex_obj, json_type_object)) return;
    
    json_object* path_obj;
    json_object_object_get_ex(tex_obj, "path", &path_obj);
    
    if (path_obj) {
        const char* path = json_object_get_string(path_obj);
        *texture = scene_load_texture(scene, path, TEXTURE_TYPE_COLOR);
    }
}

void load_sphere_config(json_object* sphere_obj, Scene* scene) {
    if (!json_object_is_type(sphere_obj, json_type_object)) return;
    
    json_object *center_obj, *radius_obj, *color_obj, *reflectivity_obj;
    json_object *fresnel_ior_obj, *fresnel_power_obj, *texture_obj;
    
    json_object_object_get_ex(sphere_obj, "center", &center_obj);
    json_object_object_get_ex(sphere_obj, "radius", &radius_obj);
    json_object_object_get_ex(sphere_obj, "color", &color_obj);
    json_object_object_get_ex(sphere_obj, "reflectivity", &reflectivity_obj);
    json_object_object_get_ex(sphere_obj, "fresnel_ior", &fresnel_ior_obj);
    json_object_object_get_ex(sphere_obj, "fresnel_power", &fresnel_power_obj);
    json_object_object_get_ex(sphere_obj, "texture", &texture_obj);
    
    Vector3 center = center_obj ? parse_vector3(center_obj) : vector_create(0, 0, 0);
    double radius = radius_obj ? json_object_get_double(radius_obj) : 1.0;
    Vector3 color = color_obj ? parse_vector3(color_obj) : vector_create(1, 1, 1);
    double reflectivity = reflectivity_obj ? json_object_get_double(reflectivity_obj) : 0.0;
    double fresnel_ior = fresnel_ior_obj ? json_object_get_double(fresnel_ior_obj) : 1.5;
    double fresnel_power = fresnel_power_obj ? json_object_get_double(fresnel_power_obj) : 1.0;
    
    Sphere sphere = sphere_create(center, radius, color, reflectivity, fresnel_ior, fresnel_power);
    
    if (texture_obj) {
        load_texture_config(texture_obj, &sphere.color_texture, scene);
    }
    
    scene_add_sphere(scene, sphere);
}

void load_light_config(json_object* light_obj, Scene* scene) {
    if (!json_object_is_type(light_obj, json_type_object)) return;
    
    json_object *position_obj, *color_obj, *intensity_obj, *radius_obj, *type_obj;
    
    json_object_object_get_ex(light_obj, "position", &position_obj);
    json_object_object_get_ex(light_obj, "color", &color_obj);
    json_object_object_get_ex(light_obj, "intensity", &intensity_obj);
    json_object_object_get_ex(light_obj, "radius", &radius_obj);
    json_object_object_get_ex(light_obj, "type", &type_obj);
    
    Vector3 position = position_obj ? parse_vector3(position_obj) : vector_create(0, 5, 0);
    Vector3 color = color_obj ? parse_vector3(color_obj) : vector_create(1, 1, 1);
    double intensity = intensity_obj ? json_object_get_double(intensity_obj) : 1.0;
    double radius = radius_obj ? json_object_get_double(radius_obj) : 0.0;
    
    Light light;
    if (type_obj && strcmp(json_object_get_string(type_obj), "area") == 0) {
        light = area_light_create(position, color, intensity, radius);
    } else {
        light = light_create(position, color, intensity);
    }
    
    scene_add_light(scene, light);
}

AnimationTrack* load_animation_track_config(json_object* anim_obj) {
    if (!json_object_is_type(anim_obj, json_type_object)) return NULL;
    
    AnimationTrack* track = animation_track_create();
    if (!track) return NULL;
    
    json_object* keyframes_obj;
    json_object_object_get_ex(anim_obj, "keyframes", &keyframes_obj);
    
    if (keyframes_obj && json_object_is_type(keyframes_obj, json_type_array)) {
        int num_keyframes = json_object_array_length(keyframes_obj);
        
        for (int i = 0; i < num_keyframes; i++) {
            json_object* kf_obj = json_object_array_get_idx(keyframes_obj, i);
            json_object *time_obj, *position_obj, *rotation_obj, *scale_obj;
            
            json_object_object_get_ex(kf_obj, "time", &time_obj);
            json_object_object_get_ex(kf_obj, "position", &position_obj);
            json_object_object_get_ex(kf_obj, "rotation", &rotation_obj);
            json_object_object_get_ex(kf_obj, "scale", &scale_obj);
            
            Keyframe kf = {
                .time = time_obj ? json_object_get_double(time_obj) : 0.0,
                .position = position_obj ? parse_vector3(position_obj) : vector_create(0, 0, 0),
                .rotation = rotation_obj ? parse_vector3(rotation_obj) : vector_create(0, 0, 0),
                .scale = scale_obj ? parse_vector3(scale_obj) : vector_create(1, 1, 1)
            };
            
            animation_track_add_keyframe(track, kf);
        }
    }
    
    return track;
}

Scene* load_scene_from_config(const char* config_file) {
    json_object* root = json_object_from_file(config_file);
    if (!root) {
        fprintf(stderr, "Error: Could not load config file: %s\n", config_file);
        return NULL;
    }
    
    Scene* scene = malloc(sizeof(Scene));
    *scene = scene_create();
    
    // Load camera settings
    json_object* camera_obj;
    if (json_object_object_get_ex(root, "camera", &camera_obj)) {
        json_object *aperture_obj, *focal_distance_obj;
        json_object_object_get_ex(camera_obj, "aperture", &aperture_obj);
        json_object_object_get_ex(camera_obj, "focal_distance", &focal_distance_obj);
        
        if (aperture_obj) scene->aperture = json_object_get_double(aperture_obj);
        if (focal_distance_obj) scene->focal_distance = json_object_get_double(focal_distance_obj);
    }
    
    // Load spheres
    json_object* spheres_obj;
    if (json_object_object_get_ex(root, "spheres", &spheres_obj) &&
        json_object_is_type(spheres_obj, json_type_array)) {
        int num_spheres = json_object_array_length(spheres_obj);
        for (int i = 0; i < num_spheres; i++) {
            json_object* sphere_obj = json_object_array_get_idx(spheres_obj, i);
            load_sphere_config(sphere_obj, scene);
        }
    }
    
    // Load lights
    json_object* lights_obj;
    if (json_object_object_get_ex(root, "lights", &lights_obj) &&
        json_object_is_type(lights_obj, json_type_array)) {
        int num_lights = json_object_array_length(lights_obj);
        for (int i = 0; i < num_lights; i++) {
            json_object* light_obj = json_object_array_get_idx(lights_obj, i);
            load_light_config(light_obj, scene);
        }
    }
    
    // Load animations
    json_object* animations_obj;
    if (json_object_object_get_ex(root, "animations", &animations_obj) &&
        json_object_is_type(animations_obj, json_type_object)) {
        
        // Load sphere animations
        json_object* sphere_anims_obj;
        if (json_object_object_get_ex(animations_obj, "spheres", &sphere_anims_obj) &&
            json_object_is_type(sphere_anims_obj, json_type_array)) {
            int num_anims = json_object_array_length(sphere_anims_obj);
            for (int i = 0; i < num_anims && i < MAX_SPHERES; i++) {
                json_object* anim_obj = json_object_array_get_idx(sphere_anims_obj, i);
                scene->sphere_animations[i] = load_animation_track_config(anim_obj);
            }
        }
    }
    
    json_object_put(root);
    return scene;
}
