#include "scene_config.h"
#include "json_parser.h"
#include "xml_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to determine file format
static enum {
    FORMAT_JSON,
    FORMAT_XML,
    FORMAT_UNKNOWN
} detect_file_format(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return FORMAT_UNKNOWN;
    
    if (strcasecmp(ext, ".json") == 0) return FORMAT_JSON;
    if (strcasecmp(ext, ".xml") == 0) return FORMAT_XML;
    return FORMAT_UNKNOWN;
}

// Helper function to get number from JsonValue safely
static double get_json_number(JsonValue* value, double default_value) {
    int success;
    if (!value) return default_value;
    double result = json_get_number(value, &success);
    return success ? result : default_value;
}

static Vector3 parse_vector3_xml(XmlNode* node) {
    Vector3 vec = {0};
    if (!node) return vec;
    
    const char* x = xml_get_attribute(node, "x");
    const char* y = xml_get_attribute(node, "y");
    const char* z = xml_get_attribute(node, "z");
    
    if (x) vec.x = atof(x);
    if (y) vec.y = atof(y);
    if (z) vec.z = atof(z);
    
    return vec;
}

static Vector3 parse_vector3_json(JsonValue* value) {
    Vector3 vec = {0};
    if (!value || value->type != JSON_OBJECT) return vec;
    
    JsonObject* obj = value->value.object;
    JsonValue* x_val = json_object_get(obj, "x");
    JsonValue* y_val = json_object_get(obj, "y");
    JsonValue* z_val = json_object_get(obj, "z");
    
    int success;
    vec.x = x_val ? json_get_number(x_val, &success) : 0.0;
    vec.y = y_val ? json_get_number(y_val, &success) : 0.0;
    vec.z = z_val ? json_get_number(z_val, &success) : 0.0;
    
    return vec;
}

static void load_texture_config(JsonValue* tex_val, Texture** texture, Scene* scene) {
    if (!tex_val || tex_val->type != JSON_OBJECT) return;
    
    JsonObject* tex_obj = tex_val->value.object;
    JsonValue* path_val = json_object_get(tex_obj, "path");
    
    int success;
    const char* path = json_get_string(path_val, &success);
    if (success && path) {
        *texture = scene_load_texture(scene, path, TEXTURE_TYPE_COLOR);
    }
}

void load_sphere_config(JsonObject* obj, Scene* scene) {
    if (!obj) return;
    
    JsonValue* center_val = json_object_get(obj, "center");
    JsonValue* radius_val = json_object_get(obj, "radius");
    JsonValue* color_val = json_object_get(obj, "color");
    JsonValue* reflectivity_val = json_object_get(obj, "reflectivity");
    JsonValue* fresnel_ior_val = json_object_get(obj, "fresnel_ior");
    JsonValue* fresnel_power_val = json_object_get(obj, "fresnel_power");
    JsonValue* texture_val = json_object_get(obj, "texture");
    
    Vector3 center = center_val ? parse_vector3_json(center_val) : vector_create(0, 0, 0);
    double radius = get_json_number(radius_val, 1.0);
    Vector3 color = color_val ? parse_vector3_json(color_val) : vector_create(1, 1, 1);
    double reflectivity = get_json_number(reflectivity_val, 0.0);
    double fresnel_ior = get_json_number(fresnel_ior_val, 1.5);
    double fresnel_power = get_json_number(fresnel_power_val, 1.0);
    
    Sphere sphere = sphere_create(center, radius, color, reflectivity, fresnel_ior, fresnel_power);
    
    // Load texture if present
    if (texture_val) {
        load_texture_config(texture_val, &sphere.color_texture, scene);
    }
    
    // Load pattern configuration if present
    JsonValue* pattern_val = json_object_get(obj, "pattern");
    if (pattern_val && pattern_val->type == JSON_OBJECT) {
        JsonObject* pattern_obj = pattern_val->value.object;
        JsonValue* type_val = json_object_get(pattern_obj, "type");
        JsonValue* scale_val = json_object_get(pattern_obj, "scale");
        JsonValue* color1_val = json_object_get(pattern_obj, "color1");
        JsonValue* color2_val = json_object_get(pattern_obj, "color2");
        
        int success;
        const char* type_str = json_get_string(type_val, &success);
        if (success && type_str) {
            if (strcmp(type_str, "solid") == 0) sphere.pattern.type = PATTERN_SOLID;
            else if (strcmp(type_str, "checkerboard") == 0) sphere.pattern.type = PATTERN_CHECKERBOARD;
            else if (strcmp(type_str, "stripe") == 0) sphere.pattern.type = PATTERN_STRIPE;
            else if (strcmp(type_str, "gradient") == 0) sphere.pattern.type = PATTERN_GRADIENT;
            else {
                fprintf(stderr, "Warning: Unknown pattern type '%s', defaulting to solid\n", type_str);
                sphere.pattern.type = PATTERN_SOLID;
            }
            
            sphere.pattern.scale = get_json_number(scale_val, 1.0);
            sphere.pattern.color1 = color1_val ? parse_vector3_json(color1_val) : color;
            sphere.pattern.color2 = color2_val ? parse_vector3_json(color2_val) : vector_create(0, 0, 0);
        }
    }
    
    scene_add_sphere(scene, sphere);
}

void load_light_config(JsonObject* obj, Scene* scene) {
    if (!obj) return;
    
    JsonValue* position_val = json_object_get(obj, "position");
    JsonValue* color_val = json_object_get(obj, "color");
    JsonValue* intensity_val = json_object_get(obj, "intensity");
    JsonValue* radius_val = json_object_get(obj, "radius");
    JsonValue* type_val = json_object_get(obj, "type");
    
    Vector3 position = position_val ? parse_vector3_json(position_val) : vector_create(0, 5, 0);
    Vector3 color = color_val ? parse_vector3_json(color_val) : vector_create(1, 1, 1);
    double intensity = get_json_number(intensity_val, 1.0);
    double radius = get_json_number(radius_val, 0.0);
    
    int success;
    const char* type = json_get_string(type_val, &success);
    
    Light light;
    if (success && type && strcmp(type, "area") == 0) {
        light = area_light_create(position, color, intensity, radius);
    } else {
        light = light_create(position, color, intensity);
    }
    
    scene_add_light(scene, light);
}

AnimationTrack* load_animation_track_config(JsonObject* obj) {
    if (!obj) return NULL;
    
    AnimationTrack* track = animation_track_create();
    if (!track) return NULL;
    
    JsonValue* keyframes_val = json_object_get(obj, "keyframes");
    JsonArray* keyframes_arr = json_get_array(keyframes_val, NULL);
    
    if (keyframes_arr) {
        for (JsonArrayElement* elem = keyframes_arr->head; elem; elem = elem->next) {
            JsonValue* kf_val = elem->value;
            if (!kf_val || kf_val->type != JSON_OBJECT) continue;
            
            JsonObject* kf_obj = kf_val->value.object;
            JsonValue* time_val = json_object_get(kf_obj, "time");
            JsonValue* position_val = json_object_get(kf_obj, "position");
            JsonValue* rotation_val = json_object_get(kf_obj, "rotation");
            JsonValue* scale_val = json_object_get(kf_obj, "scale");
            
            Keyframe kf = {
                .time = get_json_number(time_val, 0.0),
                .position = position_val ? parse_vector3_json(position_val) : vector_create(0, 0, 0),
                .rotation = rotation_val ? parse_vector3_json(rotation_val) : vector_create(0, 0, 0),
                .scale = scale_val ? parse_vector3_json(scale_val) : vector_create(1, 1, 1)
            };
            
            animation_track_add_keyframe(track, kf);
        }
    }
    
    return track;
}

static Scene* load_scene_from_xml(const char* config_file) {
    XmlDocument* doc = xml_parse_file(config_file);
    if (!doc || !doc->root) {
        fprintf(stderr, "Error: Could not parse XML file: %s\n", config_file);
        if (doc) xml_free_document(doc);
        return NULL;
    }
    
    Scene* scene = malloc(sizeof(Scene));
    *scene = scene_create();
    
    // Load camera settings
    XmlNode* camera = xml_find_element(doc->root, "camera");
    if (camera) {
        const char* aperture = xml_get_attribute(camera, "aperture");
        const char* focal_distance = xml_get_attribute(camera, "focal_distance");
        
        if (aperture) scene->aperture = atof(aperture);
        if (focal_distance) scene->focal_distance = atof(focal_distance);
    }
    
    // Load spheres
    XmlNode* spheres = xml_find_element(doc->root, "spheres");
    if (spheres) {
        XmlNode* sphere = spheres->first_child;
        while (sphere) {
            if (strcmp(sphere->name, "sphere") == 0) {
                XmlNode* center = xml_find_child(sphere, "center");
                XmlNode* color = xml_find_child(sphere, "color");
                
                Sphere s = sphere_create(
                    center ? parse_vector3_xml(center) : vector_create(0, 0, 0),
                    atof(xml_get_attribute(sphere, "radius") ?: "1.0"),
                    color ? parse_vector3_xml(color) : vector_create(1, 1, 1),
                    atof(xml_get_attribute(sphere, "reflectivity") ?: "0.0"),
                    atof(xml_get_attribute(sphere, "fresnel_ior") ?: "1.5"),
                    atof(xml_get_attribute(sphere, "fresnel_power") ?: "1.0")
                );
                
                // Load pattern configuration if present
                XmlNode* pattern = xml_find_child(sphere, "pattern");
                if (pattern) {
                    const char* type = xml_get_attribute(pattern, "type");
                    if (type) {
                        if (strcmp(type, "solid") == 0) s.pattern.type = PATTERN_SOLID;
                        else if (strcmp(type, "checkerboard") == 0) s.pattern.type = PATTERN_CHECKERBOARD;
                        else if (strcmp(type, "stripe") == 0) s.pattern.type = PATTERN_STRIPE;
                        else if (strcmp(type, "gradient") == 0) s.pattern.type = PATTERN_GRADIENT;
                        else {
                            fprintf(stderr, "Warning: Unknown pattern type '%s', defaulting to solid\n", type);
                            s.pattern.type = PATTERN_SOLID;
                        }
                        
                        s.pattern.scale = atof(xml_get_attribute(pattern, "scale") ?: "1.0");
                        
                        XmlNode* color1 = xml_find_child(pattern, "color1");
                        XmlNode* color2 = xml_find_child(pattern, "color2");
                        
                        s.pattern.color1 = color1 ? parse_vector3_xml(color1) : s.color;
                        s.pattern.color2 = color2 ? parse_vector3_xml(color2) : vector_create(0, 0, 0);
                    }
                }
                
                scene_add_sphere(scene, s);
            }
            sphere = sphere->next_sibling;
        }
    }
    
    // Load lights
    XmlNode* lights = xml_find_element(doc->root, "lights");
    if (lights) {
        XmlNode* light = lights->first_child;
        while (light) {
            if (strcmp(light->name, "light") == 0) {
                XmlNode* position = xml_find_child(light, "position");
                XmlNode* color = xml_find_child(light, "color");
                
                const char* intensity_str = xml_get_attribute(light, "intensity");
                const char* radius_str = xml_get_attribute(light, "radius");
                const char* type = xml_get_attribute(light, "type");
                
                Vector3 pos = position ? parse_vector3_xml(position) : vector_create(0, 5, 0);
                Vector3 col = color ? parse_vector3_xml(color) : vector_create(1, 1, 1);
                double intensity = intensity_str ? atof(intensity_str) : 1.0;
                double radius = radius_str ? atof(radius_str) : 0.0;
                
                Light l;
                if (type && strcmp(type, "area") == 0) {
                    l = area_light_create(pos, col, intensity, radius);
                } else {
                    l = light_create(pos, col, intensity);
                }
                
                scene_add_light(scene, l);
            }
            light = light->next_sibling;
        }
    }
    
    // Load animations
    XmlNode* animations = xml_find_element(doc->root, "animations");
    if (animations) {
        XmlNode* sphere_anims = xml_find_child(animations, "spheres");
        if (sphere_anims) {
            int anim_index = 0;
            XmlNode* anim = sphere_anims->first_child;
            while (anim && anim_index < MAX_SPHERES) {
                if (strcmp(anim->name, "animation") == 0) {
                    AnimationTrack* track = animation_track_create();
                    if (track) {
                        XmlNode* keyframe = anim->first_child;
                        while (keyframe) {
                            if (strcmp(keyframe->name, "keyframe") == 0) {
                                const char* time_str = xml_get_attribute(keyframe, "time");
                                XmlNode* position = xml_find_child(keyframe, "position");
                                XmlNode* rotation = xml_find_child(keyframe, "rotation");
                                XmlNode* scale = xml_find_child(keyframe, "scale");
                                
                                Keyframe kf = {
                                    .time = time_str ? atof(time_str) : 0.0,
                                    .position = position ? parse_vector3_xml(position) : vector_create(0, 0, 0),
                                    .rotation = rotation ? parse_vector3_xml(rotation) : vector_create(0, 0, 0),
                                    .scale = scale ? parse_vector3_xml(scale) : vector_create(1, 1, 1)
                                };
                                
                                animation_track_add_keyframe(track, kf);
                            }
                            keyframe = keyframe->next_sibling;
                        }
                        scene->sphere_animations[anim_index++] = track;
                    }
                }
                anim = anim->next_sibling;
            }
        }
    }
    
    xml_free_document(doc);
    return scene;
}

static Scene* load_scene_from_json(const char* config_file) {
    FILE* fp = fopen(config_file, "r");
    if (!fp) {
        fprintf(stderr, "Error: Could not open config file: %s\n", config_file);
        return NULL;
    }
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char* content = (char*)malloc(size + 1);
    if (!content) {
        fclose(fp);
        return NULL;
    }
    
    size_t bytes_read = fread(content, 1, size, fp);
    if (bytes_read < (size_t)size) {
        fprintf(stderr, "Warning: Could not read entire file\n");
    }
    content[bytes_read] = '\0';
    fclose(fp);
    
    char* error = NULL;
    JsonValue* root = json_parse(content, &error);
    free(content);
    
    if (!root) {
        fprintf(stderr, "Error parsing JSON: %s\n", error ? error : "unknown error");
        free(error);
        return NULL;
    }
    
    if (root->type != JSON_OBJECT) {
        fprintf(stderr, "Error: Root JSON value is not an object\n");
        json_free(root);
        return NULL;
    }
    
    Scene* scene = malloc(sizeof(Scene));
    *scene = scene_create();
    
    JsonObject* root_obj = root->value.object;
    
    // Load camera settings
    JsonValue* camera_val = json_object_get(root_obj, "camera");
    if (camera_val && camera_val->type == JSON_OBJECT) {
        JsonObject* camera_obj = camera_val->value.object;
        JsonValue* aperture_val = json_object_get(camera_obj, "aperture");
        JsonValue* focal_distance_val = json_object_get(camera_obj, "focal_distance");
        
        scene->aperture = get_json_number(aperture_val, scene->aperture);
        scene->focal_distance = get_json_number(focal_distance_val, scene->focal_distance);
    }
    
    // Load spheres
    JsonValue* spheres_val = json_object_get(root_obj, "spheres");
    JsonArray* spheres_arr = json_get_array(spheres_val, NULL);
    if (spheres_arr) {
        for (JsonArrayElement* elem = spheres_arr->head; elem; elem = elem->next) {
            if (elem->value && elem->value->type == JSON_OBJECT) {
                load_sphere_config(elem->value->value.object, scene);
            }
        }
    }
    
    // Load lights
    JsonValue* lights_val = json_object_get(root_obj, "lights");
    JsonArray* lights_arr = json_get_array(lights_val, NULL);
    if (lights_arr) {
        for (JsonArrayElement* elem = lights_arr->head; elem; elem = elem->next) {
            if (elem->value && elem->value->type == JSON_OBJECT) {
                load_light_config(elem->value->value.object, scene);
            }
        }
    }
    
    // Load animations
    JsonValue* animations_val = json_object_get(root_obj, "animations");
    if (animations_val && animations_val->type == JSON_OBJECT) {
        JsonObject* animations_obj = animations_val->value.object;
        JsonValue* sphere_anims_val = json_object_get(animations_obj, "spheres");
        JsonArray* sphere_anims_arr = json_get_array(sphere_anims_val, NULL);
        
        if (sphere_anims_arr) {
            int anim_index = 0;
            for (JsonArrayElement* elem = sphere_anims_arr->head; 
                 elem && anim_index < MAX_SPHERES; 
                 elem = elem->next, anim_index++) {
                if (elem->value && elem->value->type == JSON_OBJECT) {
                    scene->sphere_animations[anim_index] = 
                        load_animation_track_config(elem->value->value.object);
                }
            }
        }
    }
    
    json_free(root);
    return scene;
}

Scene* load_scene_from_config(const char* config_file) {
    if (!config_file) {
        fprintf(stderr, "Error: No config file provided\n");
        return NULL;
    }
    
    switch (detect_file_format(config_file)) {
        case FORMAT_JSON:
            return load_scene_from_json(config_file);
        case FORMAT_XML:
            return load_scene_from_xml(config_file);
        default:
            fprintf(stderr, "Error: Unsupported file format for %s\n", config_file);
            return NULL;
    }
}
