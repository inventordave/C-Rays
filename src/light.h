#ifndef LIGHT_H
#define LIGHT_H

#include "vector.h"

typedef struct {
    Vector3 position;
    Vector3 color;
    double intensity;
    double radius;  // Radius for area light sampling
} Light;

Light light_create(Vector3 position, Vector3 color, double intensity);
Light area_light_create(Vector3 position, Vector3 color, double intensity, double radius);
Vector3 light_random_position(Light light);

#endif
