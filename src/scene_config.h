#ifndef SCENE_CONFIG_H
#define SCENE_CONFIG_H

#include "scene.h"
#include "json_parser.h"

// Function to load scene from configuration file (JSON or XML)
Scene* load_scene_from_config(const char* config_file);

// Function to load sphere configuration
void load_sphere_config(JsonObject* sphere_obj, Scene* scene);

// Function to load light configuration
void load_light_config(JsonObject* light_obj, Scene* scene);

// Function to load animation track configuration
AnimationTrack* load_animation_track_config(JsonObject* anim_obj);

#endif
