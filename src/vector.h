#ifndef VECTOR_H
#define VECTOR_H

#include "stringy.h"

// Precision mode enumeration
typedef enum {
    PRECISION_DOUBLE,    // Standard double precision
    PRECISION_ARBITRARY  // Arbitrary precision using APDecimal
} PrecisionMode;

// Double-based vector2 representation
typedef struct {
    double u, v;
} Vector2Double;

// String-based vector2 representation
typedef struct {
    StringHandle* u;
    StringHandle* v;
} Vector2String;

// Vector2Double operations
Vector2Double vector2_double_create(double u, double v);
Vector2Double vector2_double_add(Vector2Double a, Vector2Double b);
Vector2Double vector2_double_subtract(Vector2Double a, Vector2Double b);
Vector2Double vector2_double_multiply(Vector2Double v, double scalar);
Vector2Double vector2_double_divide(Vector2Double v, double scalar);
double vector2_double_dot(Vector2Double a, Vector2Double b);
double vector2_double_length(Vector2Double v);
Vector2Double vector2_double_normalize(Vector2Double v);

// Vector2String operations
Vector2String vector2_string_create(const char* u, const char* v);
void vector2_string_free(Vector2String* v);
Vector2String vector2_string_add(const Vector2String* a, const Vector2String* b);
Vector2String vector2_string_subtract(const Vector2String* a, const Vector2String* b);
Vector2String vector2_string_multiply(const Vector2String* v, const char* scalar);
Vector2String vector2_string_divide(const Vector2String* v, const char* scalar);
char* vector2_string_dot(const Vector2String* a, const Vector2String* b);
char* vector2_string_length(const Vector2String* v);
Vector2String vector2_string_normalize(const Vector2String* v);

// Conversion functions
Vector2Double vector2_string_to_double(const Vector2String* v);
Vector2String vector2_double_to_string(const Vector2Double* v);

// String-based vector representation
typedef struct {
    StringHandle* x;
    StringHandle* y;
    StringHandle* z;
} Vector3String;

// Double-based vector representation
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

// String-based vector operations
Vector3String vector3_string_create(const char* x, const char* y, const char* z);
void vector3_string_free(Vector3String* v);
Vector3String vector3_string_add(const Vector3String* a, const Vector3String* b);
Vector3String vector3_string_subtract(const Vector3String* a, const Vector3String* b);
Vector3String vector3_string_multiply(const Vector3String* v, const char* scalar);
Vector3String vector3_string_divide(const Vector3String* v, const char* scalar);
char* vector3_string_dot(const Vector3String* a, const Vector3String* b);
Vector3String vector3_string_cross(const Vector3String* a, const Vector3String* b);
char* vector3_string_length(const Vector3String* v);
Vector3String vector3_string_normalize(const Vector3String* v);
Vector3String vector3_string_reflect(const Vector3String* v, const Vector3String* normal);

// Conversion functions
Vector3 vector3_string_to_double(const Vector3String* v);
Vector3String vector3_double_to_string(const Vector3* v);

#endif
