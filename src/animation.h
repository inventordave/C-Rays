#ifndef ANIMATION_H
#define ANIMATION_H

#include "vector.h"

// Keyframe structure for animation
typedef struct {
    double time;          // Time point for this keyframe
    Vector3 position;     // Position at this keyframe
    Vector3 rotation;     // Rotation at this keyframe
    Vector3 scale;        // Scale at this keyframe
    Vector3 velocity;     // Velocity vector for motion blur
} Keyframe;

// Animation track for a single object
typedef struct {
    Keyframe* keyframes;  // Array of keyframes
    int keyframe_count;   // Number of keyframes
    int max_keyframes;    // Capacity of keyframe array
    double duration;      // Total duration of animation
} AnimationTrack;

// Animation system state
typedef struct {
    double current_time;  // Current animation time
    double time_step;     // Time step between frames
    double frame_rate;    // Frames per second
    int current_frame;    // Current frame number
} AnimationState;

// Function declarations
AnimationTrack* animation_track_create(void);
void animation_track_destroy(AnimationTrack* track);
void animation_track_add_keyframe(AnimationTrack* track, Keyframe keyframe);
Keyframe animation_track_interpolate(AnimationTrack* track, double time);
void animation_update_state(AnimationState* state);
AnimationState animation_state_create(double frame_rate);

#endif
