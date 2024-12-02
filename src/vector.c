#include "vector.h"
#include <math.h>

Vector3 vector_create(double x, double y, double z) {
    Vector3 v = {x, y, z};
    return v;
}

Vector3 vector_add(Vector3 a, Vector3 b) {
    return vector_create(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3 vector_subtract(Vector3 a, Vector3 b) {
    return vector_create(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector3 vector_multiply(Vector3 v, double scalar) {
    return vector_create(v.x * scalar, v.y * scalar, v.z * scalar);
}

Vector3 vector_multiply_vec(Vector3 a, Vector3 b) {
    return vector_create(a.x * b.x, a.y * b.y, a.z * b.z);
}

Vector3 vector_divide(Vector3 v, double scalar) {
    return vector_create(v.x / scalar, v.y / scalar, v.z / scalar);
}

double vector_dot(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 vector_cross(Vector3 a, Vector3 b) {
    return vector_create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

double vector_length(Vector3 v) {
    return sqrt(vector_dot(v, v));
}

Vector3 vector_normalize(Vector3 v) {
    double length = vector_length(v);
    if (length == 0) return v;
    return vector_divide(v, length);
}

Vector3 vector_reflect(Vector3 v, Vector3 normal) {
    double dot = vector_dot(v, normal);
    Vector3 reflection = vector_subtract(v, vector_multiply(normal, 2.0 * dot));
    return reflection;
}
