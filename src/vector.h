#ifndef VECTOR_H
#define VECTOR_H

// Precision mode enumeration
typedef enum {
    PRECISION_DOUBLE,    // Standard double precision
    PRECISION_ARBITRARY  // Arbitrary precision using APDecimal
} PrecisionMode;

typedef struct {
    double u, v;
} Vector2;

// Vector2 operations
Vector2 vector2_create(double u, double v);
Vector2 vector2_add(Vector2 a, Vector2 b);
Vector2 vector2_subtract(Vector2 a, Vector2 b);
Vector2 vector2_multiply(Vector2 v, double scalar);
Vector2 vector2_divide(Vector2 v, double scalar);
double vector2_dot(Vector2 a, Vector2 b);
double vector2_length(Vector2 v);
Vector2 vector2_normalize(Vector2 v);

// String-based vector representation
typedef struct {
    char* x;
    char* y;
    char* z;
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
