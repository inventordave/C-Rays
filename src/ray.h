#ifndef RAY_H
#define RAY_H

#include "vector.h"

typedef struct {
    Vector3 origin;
    Vector3 direction;
} Ray;

Ray ray_create(Vector3 origin, Vector3 direction);
Vector3 ray_point_at(Ray r, double t);

#endif
