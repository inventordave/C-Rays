#ifndef SPHERE_H
#define SPHERE_H

#include "common.h"
#include "ray.h"
#include <stddef.h>

// Pattern type enumeration
typedef enum {
    PATTERN_SOLID,
    PATTERN_CHECKERBOARD,
    PATTERN_STRIPE,
    PATTERN_GRADIENT,
    PATTERN_PERLIN_NOISE,
    PATTERN_MARBLE,
    PATTERN_WOOD
} PatternType;

// Pattern structure definition
typedef struct {
    PatternType type;
    double scale;
    Vector3 color1;    // Primary color
    Vector3 color2;    // Secondary color
} Pattern;

// Sphere structure definition
typedef struct Sphere {
    Vector3 center;
    double radius;
    Vector3 color;
    double reflectivity;
    double fresnel_ior;    // Index of Refraction for Fresnel calculations
    double fresnel_power;  // Controls strength of Fresnel effect
    double dispersion;     // Controls chromatic aberration strength
    double glossiness;     // Controls the sharpness of reflections (0-1)
    double roughness;      // Surface roughness for microfacet BRDF
    double metallic;       // Metallic factor for PBR
    Texture* color_texture;// Color texture map
    double texture_scale;  // Texture tiling scale
    Pattern pattern;       // Material pattern
} Sphere;

// Function declarations
Vector3 sample_texture(Vector2Double tex_coord, Texture* texture);
Vector2Double calculate_sphere_uv(Vector3 point, Vector3 center, double scale);

struct Sphere sphere_create(Vector3 center, double radius, Vector3 color, double reflectivity, 
                          double fresnel_ior, double fresnel_power);
int sphere_intersect(struct Sphere* sphere, Ray ray, double t_min, double t_max, Hit* hit);

#endif
