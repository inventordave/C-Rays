#ifndef SHAPE_H
#define SHAPE_H

#include "vector.h"
#include "ray.h"

// Forward declarations
struct Hit;

// Shape type enumeration
typedef enum {
    SHAPE_SPHERE,
    SHAPE_PLANE,
    SHAPE_CYLINDER,
    SHAPE_CONE,
    SHAPE_MESH
} ShapeType;

// Base shape properties
typedef struct {
    ShapeType type;
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    Vector3 color;
    double reflectivity;
} ShapeProperties;

// Shape interface
typedef struct Shape {
    ShapeProperties properties;
    // Function pointer for shape-specific intersection test
    int (*intersect)(struct Shape* shape, Ray ray, double t_min, double t_max, struct Hit* hit);
    // Function pointer for shape-specific normal calculation
    Vector3 (*calculate_normal)(struct Shape* shape, Vector3 point);
} Shape;

// Shape interface functions
Shape* shape_create(ShapeType type, Vector3 position, Vector3 rotation, Vector3 scale, 
                   Vector3 color, double reflectivity);
void shape_destroy(Shape* shape);
int shape_intersect(Shape* shape, Ray ray, double t_min, double t_max, struct Hit* hit);
Vector3 shape_get_normal(Shape* shape, Vector3 point);

#endif
