#include "vector.h"
#include "gc.h"
#include "aplib.h"
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define EPSILON 1e-8
#define MIN_DENOMINATOR 1e-10

// Global precision mode setting
static PrecisionMode current_precision_mode = PRECISION_DOUBLE;

void vector_set_precision_mode(PrecisionMode mode) {
    current_precision_mode = mode;
}

PrecisionMode vector_get_precision_mode(void) {
    return current_precision_mode;
}

// String-based vector implementation
Vector3String vector3_string_create(const char* x, const char* y, const char* z) {
    Vector3String v;
    v.x = string_create(x ? x : "0.0");
    v.y = string_create(y ? y : "0.0");
    v.z = string_create(z ? z : "0.0");
    return v;
}

void vector3_string_free(Vector3String* v) {
    if (v) {
        string_release(v->x);
        string_release(v->y);
        string_release(v->z);
        v->x = v->y = v->z = NULL;
    }
}

Vector3String vector3_string_add(const Vector3String* a, const Vector3String* b) {
    char result_x[64], result_y[64], result_z[64];
    Vector3 va = vector3_string_to_double(a);
    Vector3 vb = vector3_string_to_double(b);
    Vector3 result = aplib_vector_add(va, vb);
    snprintf(result_x, sizeof(result_x), "%.20f", result.x);
    snprintf(result_y, sizeof(result_y), "%.20f", result.y);
    snprintf(result_z, sizeof(result_z), "%.20f", result.z);
    return vector3_string_create(result_x, result_y, result_z);
}

// Implement all string-based vector operations
Vector3String vector3_string_subtract(const Vector3String* a, const Vector3String* b) {
    Vector3 va = vector3_string_to_double(a);
    Vector3 vb = vector3_string_to_double(b);
    Vector3 result = aplib_vector_subtract(va, vb);
    return vector3_double_to_string(&result);
}

Vector3String vector3_string_multiply(const Vector3String* v, const char* scalar) {
    Vector3 vd = vector3_string_to_double(v);
    Vector3 result = aplib_vector_multiply(vd, scalar);
    return vector3_double_to_string(&result);
}

Vector3String vector3_string_divide(const Vector3String* v, const char* scalar) {
    Vector3 vd = vector3_string_to_double(v);
    Vector3 result = aplib_vector_divide(vd, scalar);
    return vector3_double_to_string(&result);
}

char* vector3_string_dot(const Vector3String* a, const Vector3String* b) {
    Vector3 va = vector3_string_to_double(a);
    Vector3 vb = vector3_string_to_double(b);
    double result = aplib_vector_dot(va, vb);
    char* str_result = (char*)g(malloc(64));
    snprintf(str_result, 64, "%.20f", result);
    return str_result;
}

Vector3String vector3_string_cross(const Vector3String* a, const Vector3String* b) {
    Vector3 va = vector3_string_to_double(a);
    Vector3 vb = vector3_string_to_double(b);
    Vector3 result = aplib_vector_cross(va, vb);
    return vector3_double_to_string(&result);
}

char* vector3_string_length(const Vector3String* v) {
    Vector3 vd = vector3_string_to_double(v);
    double result = aplib_vector_length(vd);
    char* str_result = (char*)g(malloc(64));
    snprintf(str_result, 64, "%.20f", result);
    return str_result;
}

Vector3String vector3_string_normalize(const Vector3String* v) {
    Vector3 vd = vector3_string_to_double(v);
    Vector3 result = aplib_vector_normalize(vd);
    return vector3_double_to_string(&result);
}

Vector3String vector3_string_reflect(const Vector3String* v, const Vector3String* normal) {
    Vector3 vd = vector3_string_to_double(v);
    Vector3 nd = vector3_string_to_double(normal);
    Vector3 result = aplib_vector_reflect(vd, nd);
    return vector3_double_to_string(&result);
}

Vector3 vector3_string_to_double(const Vector3String* v) {
    Vector3 result;
    result.x = atof(string_get(v->x));
    result.y = atof(string_get(v->y));
    result.z = atof(string_get(v->z));
    return result;
}

Vector3String vector3_double_to_string(const Vector3* v) {
    char x[64], y[64], z[64];
    snprintf(x, sizeof(x), "%.20f", v->x);
    snprintf(y, sizeof(y), "%.20f", v->y);
    snprintf(z, sizeof(z), "%.20f", v->z);
    return vector3_string_create(x, y, z);
}

Vector3 vector_create(double x, double y, double z) {
    if (current_precision_mode == PRECISION_ARBITRARY) {
        char x_str[64], y_str[64], z_str[64];
        snprintf(x_str, sizeof(x_str), "%.20f", x);
        snprintf(y_str, sizeof(y_str), "%.20f", y);
        snprintf(z_str, sizeof(z_str), "%.20f", z);
        Vector3String str_vec = vector3_string_create(x_str, y_str, z_str);
        Vector3 result = vector3_string_to_double(&str_vec);
        vector3_string_free(&str_vec);
        return result;
    }
    Vector3 v = {x, y, z};
    return v;
}

Vector3 vector_add(Vector3 a, Vector3 b) {
    if (current_precision_mode == PRECISION_ARBITRARY) {
        return aplib_vector_add(a, b);
    }
    return vector_create(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3 vector_subtract(Vector3 a, Vector3 b) {
    if (current_precision_mode == PRECISION_ARBITRARY) {
        return aplib_vector_subtract(a, b);
    }
    return vector_create(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector3 vector_multiply(Vector3 v, double scalar) {
    if (current_precision_mode == PRECISION_ARBITRARY) {
        char scalar_str[32];
        snprintf(scalar_str, sizeof(scalar_str), "%.20f", scalar);
        return aplib_vector_multiply(v, scalar_str);
    }
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
    if (current_precision_mode == PRECISION_ARBITRARY) {
        return aplib_vector_dot(a, b);
    }
    // Use fma for higher precision dot product
    return fma(a.x, b.x, fma(a.y, b.y, a.z * b.z));
}

Vector3 vector_cross(Vector3 a, Vector3 b) {
    if (current_precision_mode == PRECISION_ARBITRARY) {
        return aplib_vector_cross(a, b);
    }
    return vector_create(
        fma(a.y, b.z, -a.z * b.y),
        fma(a.z, b.x, -a.x * b.z),
        fma(a.x, b.y, -a.y * b.x)
    );
}

double vector_length(Vector3 v) {
    if (current_precision_mode == PRECISION_ARBITRARY) {
        return aplib_vector_length(v);
    }
    double dot = vector_dot(v, v);
    return sqrt(fmax(0.0, dot));  // Prevent negative sqrt input
}

Vector3 vector_normalize(Vector3 v) {
    if (current_precision_mode == PRECISION_ARBITRARY) {
        return aplib_vector_normalize(v);
    }
    double length = vector_length(v);
    return vector_safe_divide(v, length, v);
}

Vector3 vector_reflect(Vector3 v, Vector3 normal) {
    if (current_precision_mode == PRECISION_ARBITRARY) {
        return aplib_vector_reflect(v, normal);
    }
    double dot = vector_dot(v, normal);
    Vector3 scaled = vector_multiply_precise(normal, 2.0 * dot);
    return vector_subtract(v, scaled);
}
