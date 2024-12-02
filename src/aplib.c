#include "aplib.h"
#include <stdlib.h>
// Garbage collection system
#include <stddef.h>

#define MAX_TRACKED_POINTERS 1000

typedef struct {
    void* ptr;
    size_t size;
    int marked;
} GCObject;

static GCObject gc_objects[MAX_TRACKED_POINTERS];
static int gc_object_count = 0;
static int gc_initialized = 0;

void gc_init(void) {
    if (!gc_initialized) {
        gc_object_count = 0;
        gc_initialized = 1;
    }
}

void* gc_malloc(size_t size) {
    if (!gc_initialized) gc_init();
    
    void* ptr = malloc(size);
    if (ptr && gc_object_count < MAX_TRACKED_POINTERS) {
        gc_objects[gc_object_count].ptr = ptr;
        gc_objects[gc_object_count].size = size;
        gc_objects[gc_object_count].marked = 0;
        gc_object_count++;
    }
    return ptr;
}

void gc_free(void* ptr) {
    if (!ptr) return;
    
    for (int i = 0; i < gc_object_count; i++) {
        if (gc_objects[i].ptr == ptr) {
            free(ptr);
            // Remove from tracking by shifting remaining elements
            for (int j = i; j < gc_object_count - 1; j++) {
                gc_objects[j] = gc_objects[j + 1];
            }
            gc_object_count--;
            break;
        }
    }
}

void gc_mark(void* ptr) {
    if (!ptr) return;
    
    for (int i = 0; i < gc_object_count; i++) {
        if (gc_objects[i].ptr == ptr) {
            gc_objects[i].marked = 1;
            break;
        }
    }
}

void gc_collect(void) {
    for (int i = 0; i < gc_object_count; i++) {
        if (!gc_objects[i].marked) {
            void* ptr = gc_objects[i].ptr;
            gc_free(ptr);
            i--; // Adjust index since array was shifted in gc_free
        }
    }
    // Reset marks
    for (int i = 0; i < gc_object_count; i++) {
        gc_objects[i].marked = 0;
    }
}

#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_DECIMAL_DIGITS 100
#define DECIMAL_PLACES 20

typedef struct {
    char digits[MAX_DECIMAL_DIGITS];
    int decimal_point;
    int is_negative;
} APDecimal;

// Helper functions
static void normalize_decimal(APDecimal* num) {
    int len = strlen(num->digits);
    // Remove trailing zeros after decimal point
    while (len > num->decimal_point && num->digits[len-1] == '0') {
        num->digits[len-1] = '\0';
        len--;
    }
    // Remove leading zeros
    int start = 0;
    while (num->digits[start] == '0' && start < len-1) start++;
    if (start > 0) {
        memmove(num->digits, num->digits + start, len - start + 1);
        num->decimal_point -= start;
    }
}

static APDecimal str_to_decimal(const char* str) {
    APDecimal result;
    memset(&result, 0, sizeof(APDecimal));
    
    int i = 0;
    // Handle sign
    if (str[0] == '-') {
        result.is_negative = 1;
        i++;
    }
    
    int j = 0;
    result.decimal_point = -1;
    
    // Parse digits
    while (str[i]) {
        if (str[i] == '.') {
            result.decimal_point = j;
        } else if (isdigit(str[i])) {
            result.digits[j++] = str[i];
        }
        i++;
    }
    result.digits[j] = '\0';
    
    if (result.decimal_point == -1) {
        result.decimal_point = j;
    }
    
    normalize_decimal(&result);
    return result;
}

static double decimal_to_double(APDecimal dec) {
    char buf[MAX_DECIMAL_DIGITS + 2];  // +2 for sign and decimal point
    int i = 0;
    
    if (dec.is_negative) {
        buf[i++] = '-';
    }
    
    int len = strlen(dec.digits);
    if (dec.decimal_point > 0) {
        strncpy(buf + i, dec.digits, dec.decimal_point);
        i += dec.decimal_point;
        if (dec.decimal_point < len) {
            buf[i++] = '.';
            strcpy(buf + i, dec.digits + dec.decimal_point);
        }
    } else {
        strcpy(buf + i, "0.");
        i += 2;
        for (int j = 0; j < -dec.decimal_point; j++) {
            buf[i++] = '0';
        }
        strcpy(buf + i, dec.digits);
    }
    
    return atof(buf);
}

static APDecimal double_to_decimal(double value) {
    char buf[MAX_DECIMAL_DIGITS];
    snprintf(buf, sizeof(buf), "%.*f", DECIMAL_PLACES, value);
    return str_to_decimal(buf);
}

// Basic arithmetic operations
static APDecimal add_decimals(APDecimal a, APDecimal b) {
    // For now, convert to double, add, and convert back
    // This is temporary until we implement proper string-based arithmetic
    double result = decimal_to_double(a) + decimal_to_double(b);
    return double_to_decimal(result);
}

static APDecimal subtract_decimals(APDecimal a, APDecimal b) {
    double result = decimal_to_double(a) - decimal_to_double(b);
    return double_to_decimal(result);
}

static APDecimal multiply_decimals(APDecimal a, APDecimal b) {
    double result = decimal_to_double(a) * decimal_to_double(b);
    return double_to_decimal(result);
}

static APDecimal divide_decimals(APDecimal a, APDecimal b) {
    double result = decimal_to_double(a) / decimal_to_double(b);
    return double_to_decimal(result);
}

// Public API functions
Vector3 aplib_vector_create(const char* x, const char* y, const char* z) {
    APDecimal dx = str_to_decimal(x);
    APDecimal dy = str_to_decimal(y);
    APDecimal dz = str_to_decimal(z);
    return vector_create(
        decimal_to_double(dx),
        decimal_to_double(dy),
        decimal_to_double(dz)
    );
}

// Vector operations with arbitrary precision
Vector3 aplib_vector_add(Vector3 a, Vector3 b) {
    APDecimal ax = double_to_decimal(a.x);
    APDecimal ay = double_to_decimal(a.y);
    APDecimal az = double_to_decimal(a.z);
    
    APDecimal bx = double_to_decimal(b.x);
    APDecimal by = double_to_decimal(b.y);
    APDecimal bz = double_to_decimal(b.z);
    
    return vector_create(
        decimal_to_double(add_decimals(ax, bx)),
        decimal_to_double(add_decimals(ay, by)),
        decimal_to_double(add_decimals(az, bz))
    );
}

Vector3 aplib_vector_subtract(Vector3 a, Vector3 b) {
    APDecimal ax = double_to_decimal(a.x);
    APDecimal ay = double_to_decimal(a.y);
    APDecimal az = double_to_decimal(a.z);
    
    APDecimal bx = double_to_decimal(b.x);
    APDecimal by = double_to_decimal(b.y);
    APDecimal bz = double_to_decimal(b.z);
    
    return vector_create(
        decimal_to_double(subtract_decimals(ax, bx)),
        decimal_to_double(subtract_decimals(ay, by)),
        decimal_to_double(subtract_decimals(az, bz))
    );
}

Vector3 aplib_vector_multiply(Vector3 v, const char* scalar) {
    APDecimal s = str_to_decimal(scalar);
    APDecimal vx = double_to_decimal(v.x);
    APDecimal vy = double_to_decimal(v.y);
    APDecimal vz = double_to_decimal(v.z);
    
    return vector_create(
        decimal_to_double(multiply_decimals(vx, s)),
        decimal_to_double(multiply_decimals(vy, s)),
        decimal_to_double(multiply_decimals(vz, s))
    );
}

Vector3 aplib_vector_divide(Vector3 v, const char* scalar) {
    APDecimal s = str_to_decimal(scalar);
    APDecimal vx = double_to_decimal(v.x);
    APDecimal vy = double_to_decimal(v.y);
    APDecimal vz = double_to_decimal(v.z);
    
    return vector_create(
        decimal_to_double(divide_decimals(vx, s)),
        decimal_to_double(divide_decimals(vy, s)),
        decimal_to_double(divide_decimals(vz, s))
    );
}

double aplib_vector_dot(Vector3 a, Vector3 b) {
    APDecimal ax = double_to_decimal(a.x);
    APDecimal ay = double_to_decimal(a.y);
    APDecimal az = double_to_decimal(a.z);
    
    APDecimal bx = double_to_decimal(b.x);
    APDecimal by = double_to_decimal(b.y);
    APDecimal bz = double_to_decimal(b.z);
    
    APDecimal dx = multiply_decimals(ax, bx);
    APDecimal dy = multiply_decimals(ay, by);
    APDecimal dz = multiply_decimals(az, bz);
    
    return decimal_to_double(add_decimals(add_decimals(dx, dy), dz));
}

Vector3 aplib_vector_cross(Vector3 a, Vector3 b) {
    APDecimal ax = double_to_decimal(a.x);
    APDecimal ay = double_to_decimal(a.y);
    APDecimal az = double_to_decimal(a.z);
    
    APDecimal bx = double_to_decimal(b.x);
    APDecimal by = double_to_decimal(b.y);
    APDecimal bz = double_to_decimal(b.z);
    
    return vector_create(
        decimal_to_double(subtract_decimals(
            multiply_decimals(ay, bz),
            multiply_decimals(az, by)
        )),
        decimal_to_double(subtract_decimals(
            multiply_decimals(az, bx),
            multiply_decimals(ax, bz)
        )),
        decimal_to_double(subtract_decimals(
            multiply_decimals(ax, by),
            multiply_decimals(ay, bx)
        ))
    );
}

double aplib_vector_length(Vector3 v) {
    double dot = aplib_vector_dot(v, v);
    char buf[32];
    snprintf(buf, sizeof(buf), "%f", dot);
    APDecimal dec = str_to_decimal(buf);
    // TODO: Implement proper arbitrary-precision square root
    return sqrt(decimal_to_double(dec));
}

Vector3 aplib_vector_normalize(Vector3 v) {
    double length = aplib_vector_length(v);
    char length_str[32];
    snprintf(length_str, sizeof(length_str), "%f", length);
    return aplib_vector_divide(v, length_str);
}

Vector3 aplib_vector_reflect(Vector3 v, Vector3 normal) {
    double dot = aplib_vector_dot(v, normal);
    char dot_str[32];
    snprintf(dot_str, sizeof(dot_str), "%f", 2.0 * dot);
    Vector3 scaled = aplib_vector_multiply(normal, dot_str);
    return aplib_vector_subtract(v, scaled);
}