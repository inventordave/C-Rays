#include "sphere.h"
#include "vector.h"
#include "scene.h"  // For Texture type
#include <math.h>

// Create a new sphere
Sphere sphere_create(Vector3 center, double radius, Vector3 color, double reflectivity, 
                    double fresnel_ior, double fresnel_power) {
    Sphere sphere = {
        .center = center,
        .radius = radius,
        .color = color,
        .reflectivity = reflectivity,
        .fresnel_ior = fresnel_ior,
        .fresnel_power = fresnel_power,
        .texture_scale = 1.0,
        .color_texture = NULL,
        .dispersion = 0.0,
        .metallic = 0.0,
        .roughness = 0.5,
        .glossiness = 0.5,
        .pattern = {
            .type = PATTERN_SOLID,
            .scale = 1.0,
            .color1 = color,
            .color2 = {0, 0, 0}
        }
    };
    return sphere;
}

// Calculate UV coordinates for sphere texture mapping
Vector2Double calculate_sphere_uv(Vector3 point, Vector3 center, double scale) {
    const double POLE_EPSILON = 1e-6;
    
    // Convert point to sphere-local coordinates
    Vector3 local = vector_subtract(point, center);
    local = vector_multiply(local, scale);
    
    // Calculate normalized direction vector
    double length = vector_length(local);
    if (length < POLE_EPSILON) {
        return vector2_double_create(0.5, 0.0);  // Handle degenerate case at center
    }
    
    Vector3 dir = vector_divide(local, length);
    
    // Calculate spherical coordinates with pole handling
    double phi = atan2(dir.z, dir.x);
    double theta = acos(fmax(-1.0 + POLE_EPSILON, fmin(1.0 - POLE_EPSILON, dir.y)));
    
    // Convert to UV coordinates with smooth pole handling
    double u = (phi + M_PI) / (2.0 * M_PI);  // U coordinate [0,1]
    
    // Apply smoothing near poles to reduce pinching
    double pole_blend = pow(sin(theta), 0.5);  // Smooth transition near poles
    double v = (theta / M_PI) * pole_blend + (0.5 * (1.0 - pole_blend));
    
    // Ensure UV coordinates are properly normalized
    u = fmod(u, 1.0);
    if (u < 0.0) u += 1.0;
    v = fmax(0.0, fmin(1.0, v));
    
    return vector2_double_create(u, v);
}

// Sample color from texture at given UV coordinates with bilinear interpolation
Vector3 sample_texture(Vector2Double tex_coord, Texture* texture) {
    if (!texture || !texture->data) {
        return vector_create(1.0, 1.0, 1.0);  // Return white if no texture
    }

    // Wrap and clamp UV coordinates
    double u = fmod(tex_coord.u, 1.0);
    double v = fmod(tex_coord.v, 1.0);
    if (u < 0.0) u += 1.0;
    if (v < 0.0) v += 1.0;

    // Convert to pixel coordinates with proper bounds
    double px = u * (texture->width - 1);
    double py = v * (texture->height - 1);

    // Get integer and fractional parts for bilinear interpolation
    int x0 = (int)px;
    int y0 = (int)py;
    int x1 = (x0 + 1) % texture->width;
    int y1 = (y0 + 1) % texture->height;
    double fx = px - x0;
    double fy = py - y0;

    // Sample the four nearest pixels
    int idx00 = (y0 * texture->width + x0) * texture->channels;
    int idx10 = (y0 * texture->width + x1) * texture->channels;
    int idx01 = (y1 * texture->width + x0) * texture->channels;
    int idx11 = (y1 * texture->width + x1) * texture->channels;

    // Bilinear interpolation for each color channel
    Vector3 color;
    for (int i = 0; i < 3; i++) {
        double c00 = texture->data[idx00 + i] / 255.0;
        double c10 = texture->data[idx10 + i] / 255.0;
        double c01 = texture->data[idx01 + i] / 255.0;
        double c11 = texture->data[idx11 + i] / 255.0;

        // Interpolate
        double c0 = c00 * (1 - fx) + c10 * fx;
        double c1 = c01 * (1 - fx) + c11 * fx;
        double c = c0 * (1 - fy) + c1 * fy;

        if (i == 0) color.x = c;
        else if (i == 1) color.y = c;
        else color.z = c;
    }

    return color;
}

// Sphere intersection test with improved precision
int sphere_intersect(Sphere* sphere, Ray ray, double t_min, double t_max, Hit* hit) {
    const double INTERSECTION_EPSILON = 1e-8;
    
    Vector3 oc = vector_subtract(ray.origin, sphere->center);
    double a = vector_dot(ray.direction, ray.direction);
    
    // Check for degenerate ray direction
    if (a < INTERSECTION_EPSILON) {
        return 0;
    }
    
    double b = vector_dot(oc, ray.direction);
    double c = vector_dot(oc, oc) - sphere->radius * sphere->radius;
    double discriminant = b*b - a*c;
    
    // Check for near-zero discriminant
    if (discriminant < INTERSECTION_EPSILON) {
        return 0;
    }
    
    double sqrt_discriminant = sqrt(discriminant);
    double inv_a = 1.0 / a;
    
    // Try both intersection points
    double temp = (-b - sqrt_discriminant) * inv_a;
    if (temp < t_max && temp > t_min) {
        hit->t = temp;
        hit->point = ray_point_at(ray, hit->t);
        
        // Calculate normal with proper normalization
        Vector3 normal = vector_subtract(hit->point, sphere->center);
        hit->normal = vector_normalize(normal);
        
        // Calculate UV coordinates
        hit->tex_coord = calculate_sphere_uv(hit->point, sphere->center, sphere->texture_scale);
        
        hit->sphere = sphere;
        return 1;
    }
    
    temp = (-b + sqrt_discriminant) * inv_a;
    if (temp < t_max && temp > t_min) {
        hit->t = temp;
        hit->point = ray_point_at(ray, hit->t);
        
        // Calculate normal with proper normalization
        Vector3 normal = vector_subtract(hit->point, sphere->center);
        hit->normal = vector_normalize(normal);
        
        // Calculate UV coordinates
        hit->tex_coord = calculate_sphere_uv(hit->point, sphere->center, sphere->texture_scale);
        
        hit->sphere = sphere;
        return 1;
    }
    
    return 0;
}
