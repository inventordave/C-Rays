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
    // Convert point to sphere-local coordinates
    Vector3 local = vector_subtract(point, center);
    local = vector_multiply(local, scale);

    // Calculate spherical coordinates
    double phi = atan2(local.z, local.x);
    double theta = acos(local.y / vector_length(local));

    // Convert to UV coordinates
    Vector2Double uv;
    uv.u = (phi + M_PI) / (2.0 * M_PI);  // U coordinate [0,1]
    uv.v = theta / M_PI;                  // V coordinate [0,1]

    return uv;
}

// Sample color from texture at given UV coordinates
Vector3 sample_texture(Vector2Double tex_coord, Texture* texture) {
    if (!texture || !texture->data) {
        return vector_create(1.0, 1.0, 1.0);  // Return white if no texture
    }

    // Calculate pixel coordinates
    int x = (int)(tex_coord.u * texture->width) % texture->width;
    int y = (int)(tex_coord.v * texture->height) % texture->height;

    // Ensure positive coordinates
    x = (x + texture->width) % texture->width;
    y = (y + texture->height) % texture->height;

    // Sample texture data
    int idx = (y * texture->width + x) * texture->channels;
    return vector_create(
        texture->data[idx] / 255.0,
        texture->data[idx + 1] / 255.0,
        texture->data[idx + 2] / 255.0
    );
}

// Sphere intersection test
int sphere_intersect(Sphere* sphere, Ray ray, double t_min, double t_max, Hit* hit) {
    Vector3 oc = vector_subtract(ray.origin, sphere->center);
    double a = vector_dot(ray.direction, ray.direction);
    double b = vector_dot(oc, ray.direction);
    double c = vector_dot(oc, oc) - sphere->radius * sphere->radius;
    double discriminant = b*b - a*c;
    
    if (discriminant > 0) {
        double temp = (-b - sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            hit->t = temp;
            hit->point = ray_point_at(ray, hit->t);
            hit->normal = vector_divide(vector_subtract(hit->point, sphere->center), sphere->radius);
            hit->sphere = sphere;
            return 1;
        }
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            hit->t = temp;
            hit->point = ray_point_at(ray, hit->t);
            hit->normal = vector_divide(vector_subtract(hit->point, sphere->center), sphere->radius);
            hit->sphere = sphere;
            return 1;
        }
    }
    return 0;
}
