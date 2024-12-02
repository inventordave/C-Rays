#ifndef LIGHT_H
#define LIGHT_H

#include "vector.h"

typedef struct {
    Vector3 position;
    Vector3 color;
    double intensity;
} Light;

Light light_create(Vector3 position, Vector3 color, double intensity);

#endif
