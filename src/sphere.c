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