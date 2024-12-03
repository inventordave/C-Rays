#ifndef SCENE_CONFIG_H
#define SCENE_CONFIG_H

#include "scene.h"
#include <json-c/json.h>

// Function to load scene from JSON configuration file
Scene* load_scene_from_config(const char* config_file);

// Function to load sphere configuration
void load_sphere_config(json_object* sphere_obj, Scene* scene);

// Function to load light configuration
void load_light_config(json_object* light_obj, Scene* scene);

// Function to load animation track configuration
AnimationTrack* load_animation_track_config(json_object* anim_obj);

#endif
