#include "sphere.h"
#include <math.h>

Sphere sphere_create(Vector3 center, double radius, Vector3 color, double reflectivity,
                    double fresnel_ior, double fresnel_power) {
    Sphere s = {
        .center = center,
        .radius = radius,
        .color = color,
        .reflectivity = reflectivity,
        .fresnel_ior = fresnel_ior,
        .fresnel_power = fresnel_power,
        .dispersion = 0.02,    // Default chromatic aberration strength
        .glossiness = 0.5,     // Default glossiness
        .roughness = 0.5,      // Default roughness
        .metallic = 0.0,       // Default non-metallic
        .color_texture = NULL, // No texture by default
        .texture_scale = 1.0   // Default texture scale
    };
    return s;
}

Vector3 sample_texture(Vector2 tex_coord, Texture* texture) {
    if (!texture || !texture->data) return vector_create(1, 1, 1);
    
    // Calculate texture coordinates in pixels
    int x = (int)((tex_coord.u * texture->width)) % texture->width;
    int y = (int)((tex_coord.v * texture->height)) % texture->height;
    
    // Ensure positive coordinates
    x = (x + texture->width) % texture->width;
    y = (y + texture->height) % texture->height;
    
    // Get color from texture
    int idx = (y * texture->width + x) * texture->channels;
    return vector_create(
        texture->data[idx] / 255.0,
        texture->data[idx + 1] / 255.0,
        texture->data[idx + 2] / 255.0
    );
}

Vector2 calculate_sphere_uv(Vector3 point, Vector3 center, double scale) {
    Vector3 normalized = vector_normalize(vector_subtract(point, center));
    Vector2 tex_coord = {
        0.5 + atan2(normalized.z, normalized.x) / (2.0 * M_PI),
        0.5 - asin(normalized.y) / M_PI
    };
    
    // Apply texture scaling
    tex_coord.u *= scale;
    tex_coord.v *= scale;
    
    return tex_coord;
}

int sphere_intersect(struct Sphere* sphere, Ray ray, double t_min, double t_max, Hit* hit) {
    Vector3 oc = vector_subtract(ray.origin, sphere->center);
    double a = vector_dot(ray.direction, ray.direction);
    double b = vector_dot(oc, ray.direction);
    double c = vector_dot(oc, oc) - sphere->radius * sphere->radius;
    double discriminant = b*b - a*c;

    if (discriminant > 0) {
        double temp = (-b - sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            hit->t = temp;
            hit->point = ray_point_at(ray, temp);
            hit->normal = vector_divide(vector_subtract(hit->point, sphere->center), sphere->radius);
            
            // Calculate texture coordinates using sphere's texture scale
            hit->tex_coord = calculate_sphere_uv(hit->point, sphere->center, sphere->texture_scale);
            
            // Set sphere reference and type
            hit->sphere = sphere;
            hit->is_mesh = 0;
            return 1;
        }
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            hit->t = temp;
            hit->point = ray_point_at(ray, temp);
            hit->normal = vector_divide(vector_subtract(hit->point, sphere->center), sphere->radius);
            
            // Calculate texture coordinates using sphere's texture scale
            hit->tex_coord = calculate_sphere_uv(hit->point, sphere->center, sphere->texture_scale);
            
            // Set sphere reference and type
            hit->sphere = sphere;
            hit->is_mesh = 0;
            return 1;
        }
    }
    return 0;
}
// Perlin Noise Implementation
static double fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static double lerp(double t, double a, double b) {
    return a + t * (b - a);
}

static double grad(int hash, double x, double y, double z) {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

static double perlin_noise(Vector3 point, double scale) {
    point = vector_multiply(point, scale);
    int X = (int)floor(point.x) & 255;
    int Y = (int)floor(point.y) & 255;
    int Z = (int)floor(point.z) & 255;

    point.x -= floor(point.x);
    point.y -= floor(point.y);
    point.z -= floor(point.z);

    double u = fade(point.x);
    double v = fade(point.y);
    double w = fade(point.z);

    static const int p[512] = { /* Permutation table */ };
    static const int permutation[256] = {
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
        140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
        247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
        57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
        74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
        60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
        65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,
        200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,
        52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,
        207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,
        119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,
        129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
        218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,
        81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,
        184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,
        222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };

    for (int i = 0; i < 256; i++) p[256 + i] = p[i] = permutation[i];

    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;

    return lerp(w, lerp(v, lerp(u, grad(p[AA], point.x, point.y, point.z),
                                  grad(p[BA], point.x - 1, point.y, point.z)),
                           lerp(u, grad(p[AB], point.x, point.y - 1, point.z),
                                  grad(p[BB], point.x - 1, point.y - 1, point.z))),
                   lerp(v, lerp(u, grad(p[AA + 1], point.x, point.y, point.z - 1),
                                  grad(p[BA + 1], point.x - 1, point.y, point.z - 1)),
                           lerp(u, grad(p[AB + 1], point.x, point.y - 1, point.z - 1),
                                  grad(p[BB + 1], point.x - 1, point.y - 1, point.z - 1))));
}

// Pattern generation functions
static Vector3 perlin_noise_pattern(Vector3 point, Pattern pattern) {
    double noise = perlin_noise(point, pattern.scale);
    noise = (noise + 1.0) * 0.5; // Map to [0,1]
    return vector_lerp(pattern.color1, pattern.color2, noise);
}

static Vector3 marble_pattern(Vector3 point, Pattern pattern) {
    double noise = perlin_noise(point, pattern.scale);
    double marble = sin(point.x * pattern.scale + noise * 5.0) * 0.5 + 0.5;
    return vector_lerp(pattern.color1, pattern.color2, marble);
}

static Vector3 wood_pattern(Vector3 point, Pattern pattern) {
    // Convert to cylindrical coordinates
    double r = sqrt(point.x * point.x + point.z * point.z);
    double noise = perlin_noise(point, pattern.scale * 0.5);
    double wood = fmod(r * pattern.scale + noise * 2.0, 1.0);
    return vector_lerp(pattern.color1, pattern.color2, wood);
}

// Function to compute pattern color based on type
Vector3 compute_pattern_color(Vector3 point, Pattern pattern) {
    switch (pattern.type) {
        case PATTERN_PERLIN_NOISE:
            return perlin_noise_pattern(point, pattern);
        case PATTERN_MARBLE:
            return marble_pattern(point, pattern);
        case PATTERN_WOOD:
            return wood_pattern(point, pattern);
        case PATTERN_SOLID:
            return pattern.color1;
        case PATTERN_CHECKERBOARD: {
            int x = (int)floor(point.x * pattern.scale);
            int y = (int)floor(point.y * pattern.scale);
            int z = (int)floor(point.z * pattern.scale);
            return ((x + y + z) % 2 == 0) ? pattern.color1 : pattern.color2;
        }
        case PATTERN_STRIPE: {
            int x = (int)floor(point.x * pattern.scale);
            return (x % 2 == 0) ? pattern.color1 : pattern.color2;
        }
        case PATTERN_GRADIENT: {
            double t = fmod(point.x * pattern.scale, 1.0);
            t = t < 0 ? t + 1.0 : t;
            return vector_lerp(pattern.color1, pattern.color2, t);
        }
        default:
            return pattern.color1;
    }
}