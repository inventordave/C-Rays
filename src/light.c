#include "light.h"

#include <stdlib.h>
#include <math.h>

Light light_create(Vector3 position, Vector3 color, double intensity) {
    Light l = {
        .position = position,
        .color = color,
        .intensity = intensity,
        .radius = 0.0,
        .width = vector_create(0, 0, 0),
        .height = vector_create(0, 0, 0),
        .light_type = LIGHT_TYPE_POINT
    };
    return l;
}

Light area_light_create(Vector3 position, Vector3 color, double intensity, double radius) {
    Light l = light_create(position, color, intensity);
    l.radius = radius;
    l.light_type = LIGHT_TYPE_CIRCULAR;
    return l;
}

Light rectangular_light_create(Vector3 position, Vector3 color, double intensity, 
                             Vector3 width, Vector3 height) {
    Light l = light_create(position, color, intensity);
    l.width = width;
    l.height = height;
    l.light_type = LIGHT_TYPE_RECTANGULAR;
    return l;
}

// Hammersley sequence for better sampling distribution
static double radical_inverse(unsigned int bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return (double)bits * 2.3283064365386963e-10;
}

Vector3 light_random_position(Light light) {
    static unsigned int sample_index = 0;
    sample_index = (sample_index + 1) % 1024; // Reset after 1024 samples
    
    switch (light.light_type) {
        case LIGHT_TYPE_POINT:
            return light.position;
            
        case LIGHT_TYPE_CIRCULAR: {
            // Use Hammersley sequence for better distribution
            double u1 = (double)sample_index / 1024.0;
            double u2 = radical_inverse(sample_index);
            
            // Concentric disk mapping for better stratification
            double r = light.radius * sqrt(u1);
            double theta = 2.0 * M_PI * u2;
            
            Vector3 offset = vector_create(
                r * cos(theta),
                0.0,
                r * sin(theta)
            );
            
            return vector_add(light.position, offset);
        }
        
        case LIGHT_TYPE_RECTANGULAR: {
            // Use Hammersley sequence for better distribution
            double u = radical_inverse(sample_index);
            double v = (double)sample_index / 1024.0;
            
            // Calculate position on rectangular area light
            Vector3 scaled_width = vector_multiply(light.width, u - 0.5);
            Vector3 scaled_height = vector_multiply(light.height, v - 0.5);
            
            return vector_add(
                vector_add(light.position, scaled_width),
                scaled_height
            );
        }
        
        default:
            return light.position;
    }
}
