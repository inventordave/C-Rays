#include "shape.h"
#include <stdlib.h>
#include <stdio.h>
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
    // TODO: Implement proper cylinder intersection
    return 0;
}

static Vector3 shape_cylinder_normal(Shape* shape, Vector3 point) {
    // TODO: Implement proper cylinder normal calculation
    return vector_normalize(point);
}

static int shape_cone_intersect(Shape* shape, Ray ray, double t_min, double t_max, struct Hit* hit) {
    // TODO: Implement proper cone intersection
    return 0;
}

static Vector3 shape_cone_normal(Shape* shape, Vector3 point) {
    // TODO: Implement proper cone normal calculation
    return vector_normalize(point);
}
