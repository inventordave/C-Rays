#ifndef LIGHT_H
#define LIGHT_H

#include "vector.h"

typedef struct {
    Vector3 position;
    Vector3 color;
    double intensity;
    double radius;      // Radius for circular area light sampling
    Vector3 width;      // Width vector for rectangular area light
    Vector3 height;     // Height vector for rectangular area light
    int light_type;     // 0 for point, 1 for circular, 2 for rectangular
} Light;

#define LIGHT_TYPE_POINT 0
#define LIGHT_TYPE_CIRCULAR 1
#define LIGHT_TYPE_RECTANGULAR 2

Light light_create(Vector3 position, Vector3 color, double intensity);
Light area_light_create(Vector3 position, Vector3 color, double intensity, double radius);
Vector3 light_random_position(Light light);

#endif
