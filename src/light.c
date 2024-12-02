#include "light.h"

#include <stdlib.h>
#include <math.h>

Light light_create(Vector3 position, Vector3 color, double intensity) {
    Light l = {position, color, intensity, 0.0};  // Point light has radius 0
    return l;
}

Light area_light_create(Vector3 position, Vector3 color, double intensity, double radius) {
    Light l = {position, color, intensity, radius};
    return l;
}

Vector3 light_random_position(Light light) {
    if (light.radius <= 0.0) return light.position;
    
    // Generate random point on disk
    double r = light.radius * sqrt((double)rand() / RAND_MAX);
    double theta = 2.0 * M_PI * ((double)rand() / RAND_MAX);
    
    Vector3 offset = vector_create(
        r * cos(theta),
        0.0,  // Keep light at same height
        r * sin(theta)
    );
    
    return vector_add(light.position, offset);
}
