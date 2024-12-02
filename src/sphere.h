#ifndef SPHERE_H
#define SPHERE_H

#include "vector.h"
#include "ray.h"

typedef struct {
    Vector3 center;
    double radius;
    Vector3 color;
    double reflectivity;
} Sphere;

typedef struct {
    double t;
    Vector3 point;
    Vector3 normal;
    Sphere* sphere;
} Hit;

Sphere sphere_create(Vector3 center, double radius, Vector3 color, double reflectivity);
int sphere_intersect(Sphere* sphere, Ray ray, double t_min, double t_max, Hit* hit);

#endif
