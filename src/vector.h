#ifndef VECTOR_H
#define VECTOR_H

// Precision mode enumeration
typedef enum {
    PRECISION_DOUBLE,    // Standard double precision
    PRECISION_ARBITRARY  // Arbitrary precision using APDecimal
} PrecisionMode;

typedef struct {
    double x, y, z;
} Vector3;

// Precision mode control
void vector_set_precision_mode(PrecisionMode mode);
PrecisionMode vector_get_precision_mode(void);

// Standard double precision operations
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

// Arbitrary precision operations
Vector3 vector_create_ap(const char* x, const char* y, const char* z);
Vector3 vector_add_ap(Vector3 a, Vector3 b);
Vector3 vector_subtract_ap(Vector3 a, Vector3 b);
Vector3 vector_multiply_ap(Vector3 v, const char* scalar);
Vector3 vector_divide_ap(Vector3 v, const char* scalar);
double vector_dot_ap(Vector3 a, Vector3 b);
Vector3 vector_cross_ap(Vector3 a, Vector3 b);
double vector_length_ap(Vector3 v);
Vector3 vector_normalize_ap(Vector3 v);
Vector3 vector_reflect_ap(Vector3 v, Vector3 normal);

#endif
