#include "animation.h"
#include <stdlib.h>
#include <math.h>

#define INITIAL_KEYFRAME_CAPACITY 16

AnimationTrack* animation_track_create(void) {
    AnimationTrack* track = (AnimationTrack*)malloc(sizeof(AnimationTrack));
    if (!track) return NULL;

    track->keyframes = (Keyframe*)malloc(INITIAL_KEYFRAME_CAPACITY * sizeof(Keyframe));
    if (!track->keyframes) {
        free(track);
        return NULL;
    }

    track->keyframe_count = 0;
    track->max_keyframes = INITIAL_KEYFRAME_CAPACITY;
    track->duration = 0.0;
    return track;
}

void animation_track_destroy(AnimationTrack* track) {
    if (track) {
        free(track->keyframes);
        free(track);
    }
}

void animation_track_add_keyframe(AnimationTrack* track, Keyframe keyframe) {
    if (!track) return;

    // Resize if needed
    if (track->keyframe_count >= track->max_keyframes) {
        int new_capacity = track->max_keyframes * 2;
        Keyframe* new_keyframes = (Keyframe*)realloc(track->keyframes, 
                                                    new_capacity * sizeof(Keyframe));
        if (!new_keyframes) return;

        track->keyframes = new_keyframes;
        track->max_keyframes = new_capacity;
    }

    // Insert keyframe in time-sorted order
    int insert_index = 0;
    while (insert_index < track->keyframe_count && 
           track->keyframes[insert_index].time < keyframe.time) {
        insert_index++;
    }

    // Shift existing keyframes
    for (int i = track->keyframe_count; i > insert_index; i--) {
        track->keyframes[i] = track->keyframes[i - 1];
    }

    track->keyframes[insert_index] = keyframe;
    track->keyframe_count++;

    // Update duration if necessary
    if (keyframe.time > track->duration) {
        track->duration = keyframe.time;
    }
}

static Vector3 lerp(Vector3 a, Vector3 b, double t) {
    return vector_add(
        vector_multiply(a, 1.0 - t),
        vector_multiply(b, t)
    );
}

// Smooth step function for better interpolation
static double smooth_step(double t) {
    return t * t * (3.0 - 2.0 * t);
}

Keyframe animation_track_interpolate(AnimationTrack* track, double time) {
    if (!track || track->keyframe_count == 0) {
        Keyframe empty = {0};
        return empty;
    }

    // Wrap time to animation duration
    time = fmod(time, track->duration);
    if (time < 0) time += track->duration;

    // Find keyframes to interpolate between
    int next_idx = 0;
    while (next_idx < track->keyframe_count && 
           track->keyframes[next_idx].time < time) {
        next_idx++;
    }

    int prev_idx = next_idx - 1;
    
    // Handle boundary cases
    if (prev_idx < 0) {
        prev_idx = track->keyframe_count - 1;
        next_idx = 0;
    }
    if (next_idx >= track->keyframe_count) {
        next_idx = 0;
    }

    Keyframe* prev = &track->keyframes[prev_idx];
    Keyframe* next = &track->keyframes[next_idx];

    // Calculate interpolation factor
    double segment_duration = next->time - prev->time;
    if (segment_duration < 0) segment_duration += track->duration;
    
    double t = (time - prev->time) / segment_duration;
    if (t < 0) t += 1.0;

    // Apply smooth interpolation
    t = smooth_step(t);

    // Interpolate all properties
    Keyframe result;
    result.time = time;
    result.position = lerp(prev->position, next->position, t);
    result.rotation = lerp(prev->rotation, next->rotation, t);
    result.scale = lerp(prev->scale, next->scale, t);
    result.velocity = vector_subtract(next->position, prev->position);
    
    // Scale velocity by time step
    result.velocity = vector_multiply(result.velocity, 1.0 / segment_duration);

    return result;
}

AnimationState animation_state_create(double frame_rate) {
    AnimationState state = {
        .current_time = 0.0,
        .time_step = 1.0 / frame_rate,
        .frame_rate = frame_rate,
        .current_frame = 0
    };
    return state;
}

void animation_update_state(AnimationState* state) {
    if (!state) return;
    state->current_time += state->time_step;
    state->current_frame++;
}
