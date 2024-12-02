#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
    double x, y, z;
} Vector3;

Vector3 vector_create(double x, double y, double z);
Vector3 vector_add(Vector3 a, Vector3 b);
Vector3 vector_subtract(Vector3 a, Vector3 b);
Vector3 vector_multiply(Vector3 v, double scalar);
Vector3 vector_multiply_precise(Vector3 v, double scalar);
Vector3 vector_multiply_vec(Vector3 a, Vector3 b);
Vector3 vector_divide(Vector3 v, double scalar);
Vector3 vector_safe_divide(Vector3 v, double scalar, Vector3 fallback);
double vector_dot(Vector3 a, Vector3 b);
Vector3 vector_cross(Vector3 a, Vector3 b);
double vector_length(Vector3 v);
Vector3 vector_normalize(Vector3 v);
Vector3 vector_reflect(Vector3 v, Vector3 normal);

#endif
