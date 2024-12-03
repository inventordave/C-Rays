#include "shape.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "sphere.h"
#include "aplib.h"
#include <string.h>

// Forward declarations for GC functions from aplib.h
void gc_init(void);
void* gc_malloc(size_t size);
void gc_free(void* ptr);
void gc_mark(void* ptr);

// Forward declarations for shape-specific functions
static int shape_sphere_intersect(Shape* shape, Ray ray, double t_min, double t_max, struct Hit* hit);
static Vector3 shape_sphere_normal(Shape* shape, Vector3 point);
static int shape_cylinder_intersect(Shape* shape, Ray ray, double t_min, double t_max, struct Hit* hit);
static Vector3 shape_cylinder_normal(Shape* shape, Vector3 point);
static int shape_cone_intersect(Shape* shape, Ray ray, double t_min, double t_max, struct Hit* hit);
static Vector3 shape_cone_normal(Shape* shape, Vector3 point);

Shape* shape_create(ShapeType type, Vector3 position, Vector3 rotation, Vector3 scale, 
                   Vector3 color, double reflectivity) {
    // Initialize garbage collector if not already initialized
    gc_init();

    Shape* shape = (Shape*)gc_malloc(sizeof(Shape));
    if (!shape) {
        fprintf(stderr, "Error: Failed to allocate memory for shape\n");
        return NULL;
    }

    // Initialize base properties
    shape->properties.type = type;
    shape->properties.position = position;
    shape->properties.rotation = rotation;
    shape->properties.scale = scale;
    shape->properties.color = color;
    shape->properties.reflectivity = reflectivity;

    // Set type-specific function pointers
    switch (type) {
        case SHAPE_SPHERE:
            shape->intersect = shape_sphere_intersect;
            shape->calculate_normal = shape_sphere_normal;
            break;
        case SHAPE_CYLINDER:
            shape->intersect = shape_cylinder_intersect;
            shape->calculate_normal = shape_cylinder_normal;
            break;
        case SHAPE_CONE:
            shape->intersect = shape_cone_intersect;
            shape->calculate_normal = shape_cone_normal;
            break;
        default:
            // Default to sphere implementation
            shape->intersect = shape_sphere_intersect;
            shape->calculate_normal = shape_sphere_normal;
    }

    // Mark the shape as in use
    gc_mark(shape);
    return shape;
}

void shape_destroy(Shape* shape) {
    if (shape) {
        gc_free(shape);
    }
}

int shape_intersect(Shape* shape, Ray ray, double t_min, double t_max, struct Hit* hit) {
    if (!shape) return 0;
    
    // Mark the shape as still in use during intersection testing
    gc_mark(shape);
    
    if (shape->intersect) {
        return shape->intersect(shape, ray, t_min, t_max, hit);
    }
    return 0;
}

Vector3 shape_get_normal(Shape* shape, Vector3 point) {
    if (shape && shape->calculate_normal) {
        return shape->calculate_normal(shape, point);
    }
    return vector_create(0, 0, 0);
}

// Shape-specific implementations
static int shape_sphere_intersect(Shape* shape, Ray ray, double t_min, double t_max, struct Hit* hit) {
    // Convert generic shape to sphere-specific parameters
    struct Sphere temp_sphere = sphere_create(
        shape->properties.position,  // center
        shape->properties.scale.x,   // radius (use x component)
        shape->properties.color,     // color
        shape->properties.reflectivity, // reflectivity
        1.5,  // default fresnel_ior for glass-like materials
        1.0   // default fresnel_power
    );
    return sphere_intersect(&temp_sphere, ray, t_min, t_max, hit);
}

static Vector3 shape_sphere_normal(Shape* shape, Vector3 point) {
    return vector_normalize(vector_subtract(point, shape->properties.position));
}

static int shape_cylinder_intersect(Shape* shape, Ray ray, double t_min, double t_max, struct Hit* hit) {
    // Transform ray to cylinder's local space
    Vector3 oc = vector_subtract(ray.origin, shape->properties.position);
    
    // Project ray onto XZ plane for infinite cylinder intersection
    double a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z;
    double b = 2.0 * (oc.x * ray.direction.x + oc.z * ray.direction.z);
    double c = oc.x * oc.x + oc.z * oc.z - shape->properties.scale.x * shape->properties.scale.x;
    
    double discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) return 0;
    
    double t = (-b - sqrt(discriminant)) / (2.0 * a);
    if (t < t_min || t > t_max) {
        t = (-b + sqrt(discriminant)) / (2.0 * a);
        if (t < t_min || t > t_max) return 0;
    }
    
    // Check cylinder height bounds
    Vector3 intersection = ray_point_at(ray, t);
    double height = intersection.y - shape->properties.position.y;
    if (fabs(height) > shape->properties.scale.y) return 0;
    
    hit->t = t;
    hit->point = intersection;
    
    // Calculate normal
    Vector3 cp = vector_subtract(intersection, shape->properties.position);
    cp.y = 0;  // Project to XZ plane
    hit->normal = vector_normalize(cp);
    
    return 1;
}

static Vector3 shape_cylinder_normal(Shape* shape, Vector3 point) {
    Vector3 cp = vector_subtract(point, shape->properties.position);
    cp.y = 0;  // Project to XZ plane for side normal
    return vector_normalize(cp);
}

static int shape_cone_intersect(Shape* shape, Ray ray, double t_min, double t_max, struct Hit* hit) {
    Vector3 oc = vector_subtract(ray.origin, shape->properties.position);
    double theta = atan(shape->properties.scale.x / shape->properties.scale.y); // Cone angle
    double tan_theta = tan(theta);
    double tan_theta2 = tan_theta * tan_theta;
    
    // Quadratic equation coefficients
    double a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z 
             - (ray.direction.y * ray.direction.y * tan_theta2);
    double b = 2.0 * (oc.x * ray.direction.x + oc.z * ray.direction.z 
             - (oc.y * ray.direction.y * tan_theta2));
    double c = oc.x * oc.x + oc.z * oc.z - (oc.y * oc.y * tan_theta2);
    
    double discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) return 0;
    
    // Find closest valid intersection
    double t = (-b - sqrt(discriminant)) / (2.0 * a);
    if (t < t_min || t > t_max) {
        t = (-b + sqrt(discriminant)) / (2.0 * a);
        if (t < t_min || t > t_max) return 0;
    }
    
    // Check height bounds
    Vector3 intersection = ray_point_at(ray, t);
    double height = intersection.y - shape->properties.position.y;
    if (height < 0 || height > shape->properties.scale.y) return 0;
    
    hit->t = t;
    hit->point = intersection;
    
    // Calculate normal
    Vector3 cp = vector_subtract(intersection, shape->properties.position);
    double r = sqrt(cp.x * cp.x + cp.z * cp.z);
    hit->normal = vector_normalize(vector_create(
        cp.x,
        -r * tan_theta,
        cp.z
    ));
    
    return 1;
}

static Vector3 shape_cone_normal(Shape* shape, Vector3 point) {
    Vector3 cp = vector_subtract(point, shape->properties.position);
    double theta = atan(shape->properties.scale.x / shape->properties.scale.y);
    double r = sqrt(cp.x * cp.x + cp.z * cp.z);
    return vector_normalize(vector_create(
        cp.x,
        -r * tan(theta),
        cp.z
    ));
}
