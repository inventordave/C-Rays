#include "ray.h"

Ray ray_create(Vector3 origin, Vector3 direction) {
    Ray r = {
        .origin = origin,
        .direction = vector_normalize(direction),
        .wavelength_offset = 0.0,
        .time = 0.0
    };
    return r;
}

Vector3 ray_point_at(Ray r, double t) {
    return vector_add(r.origin, vector_multiply(r.direction, t));
}
