#include "sphere.h"
#include <math.h>

Sphere sphere_create(Vector3 center, double radius, Vector3 color, double reflectivity) {
    Sphere s = {center, radius, color, reflectivity};
    return s;
}

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
            hit->point = ray_point_at(ray, temp);
            hit->normal = vector_divide(vector_subtract(hit->point, sphere->center), sphere->radius);
            hit->sphere = sphere;
            return 1;
        }
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            hit->t = temp;
            hit->point = ray_point_at(ray, temp);
            hit->normal = vector_divide(vector_subtract(hit->point, sphere->center), sphere->radius);
            hit->sphere = sphere;
            return 1;
        }
    }
    return 0;
}
