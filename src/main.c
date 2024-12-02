#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scene.h"

#define WIDTH 800
#define HEIGHT 600

void write_color(FILE* fp, Vector3 color) {
    int r = (int)(255.99 * fmin(1.0, fmax(0.0, color.x)));
    int g = (int)(255.99 * fmin(1.0, fmax(0.0, color.y)));
    int b = (int)(255.99 * fmin(1.0, fmax(0.0, color.z)));
    fprintf(fp, "%d %d %d\n", r, g, b);
}

int main() {
    FILE* fp = fopen("output.ppm", "w");
    if (!fp) {
        fprintf(stderr, "Error: Could not open output file\n");
        return 1;
    }

    // Initialize scene
    Scene scene = scene_create();

    // Add spheres
    scene_add_sphere(&scene, sphere_create(vector_create(0, 0, -5), 1.0, vector_create(1.0, 0.2, 0.2), 0.3));
    scene_add_sphere(&scene, sphere_create(vector_create(2, 0, -6), 1.0, vector_create(0.2, 1.0, 0.2), 0.3));
    scene_add_sphere(&scene, sphere_create(vector_create(-2, 0, -4), 1.0, vector_create(0.2, 0.2, 1.0), 0.3));
    scene_add_sphere(&scene, sphere_create(vector_create(0, -101, -5), 100.0, vector_create(0.5, 0.5, 0.5), 0.1));

    // Add lights
    scene_add_light(&scene, light_create(vector_create(5, 5, -5), vector_create(1, 1, 1), 1.0));
    scene_add_light(&scene, light_create(vector_create(-5, 5, -5), vector_create(0.5, 0.5, 0.5), 0.5));

    // Write PPM header
    fprintf(fp, "P3\n%d %d\n255\n", WIDTH, HEIGHT);

    // Camera parameters
    Vector3 camera_pos = vector_create(0, 0, 1);
    double viewport_height = 2.0;
    double viewport_width = viewport_height * (double)WIDTH / HEIGHT;
    double focal_length = 1.0;

    Vector3 horizontal = vector_create(viewport_width, 0, 0);
    Vector3 vertical = vector_create(0, viewport_height, 0);
    Vector3 lower_left_corner = vector_subtract(
        vector_subtract(
            vector_subtract(camera_pos, vector_divide(horizontal, 2.0)),
            vector_divide(vertical, 2.0)
        ),
        vector_create(0, 0, focal_length)
    );

    // Render scene
    for (int j = HEIGHT - 1; j >= 0; j--) {
        fprintf(stderr, "\rScanlines remaining: %d ", j);
        for (int i = 0; i < WIDTH; i++) {
            double u = (double)i / (WIDTH - 1);
            double v = (double)j / (HEIGHT - 1);

            Vector3 direction = vector_subtract(
                vector_add(
                    vector_add(lower_left_corner,
                        vector_multiply(horizontal, u)),
                    vector_multiply(vertical, v)
                ),
                camera_pos
            );

            Ray ray = ray_create(camera_pos, direction);
            Vector3 color = scene_trace(&scene, ray, MAX_DEPTH);
            write_color(fp, color);
        }
    }

    fprintf(stderr, "\nDone.\n");
    fclose(fp);
    return 0;
}
