#include "vector.h"
#include <math.h>
#include <float.h>

#define EPSILON 1e-8
#define MIN_DENOMINATOR 1e-10

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
    // Handle very small numbers
    if (fabs(scalar) < EPSILON) {
        return vector_create(0.0, 0.0, 0.0);
    }
    return vector_create(v.x * scalar, v.y * scalar, v.z * scalar);
}

Vector3 vector_multiply_precise(Vector3 v, double scalar) {
    // Use fma for higher precision multiplication
    double x = fma(v.x, scalar, 0.0);
    double y = fma(v.y, scalar, 0.0);
    double z = fma(v.z, scalar, 0.0);
    return vector_create(x, y, z);
}

Vector3 vector_multiply_vec(Vector3 a, Vector3 b) {
    return vector_create(
        fma(a.x, b.x, 0.0),
        fma(a.y, b.y, 0.0),
        fma(a.z, b.z, 0.0)
    );
}

Vector3 vector_divide(Vector3 v, double scalar) {
    // Prevent division by very small numbers
    if (fabs(scalar) < EPSILON) {
        scalar = scalar < 0 ? -EPSILON : EPSILON;
    }
    return vector_create(v.x / scalar, v.y / scalar, v.z / scalar);
}

Vector3 vector_safe_divide(Vector3 v, double scalar, Vector3 fallback) {
    if (fabs(scalar) < MIN_DENOMINATOR) {
        return fallback;
    }
    return vector_create(v.x / scalar, v.y / scalar, v.z / scalar);
}

double vector_dot(Vector3 a, Vector3 b) {
    // Use fma for higher precision dot product
    return fma(a.x, b.x, fma(a.y, b.y, a.z * b.z));
}

Vector3 vector_cross(Vector3 a, Vector3 b) {
    return vector_create(
        fma(a.y, b.z, -a.z * b.y),
        fma(a.z, b.x, -a.x * b.z),
        fma(a.x, b.y, -a.y * b.x)
    );
}

double vector_length(Vector3 v) {
    double dot = vector_dot(v, v);
    return sqrt(fmax(0.0, dot));  // Prevent negative sqrt input
}

Vector3 vector_normalize(Vector3 v) {
    double length = vector_length(v);
    return vector_safe_divide(v, length, v);
}

Vector3 vector_reflect(Vector3 v, Vector3 normal) {
    double dot = vector_dot(v, normal);
    Vector3 scaled = vector_multiply_precise(normal, 2.0 * dot);
    return vector_subtract(v, scaled);
}
