#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "scene.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 800
#define HEIGHT 600

typedef enum {
    FORMAT_PPM,
    FORMAT_PNG
} OutputFormat;

void write_color_ppm(FILE* fp, Vector3 color) {
    int r = (int)(255.99 * fmin(1.0, fmax(0.0, color.x)));
    int g = (int)(255.99 * fmin(1.0, fmax(0.0, color.y)));
    int b = (int)(255.99 * fmin(1.0, fmax(0.0, color.z)));
    fprintf(fp, "%d %d %d\n", r, g, b);
}

void save_png(const char* filename, Vector3* pixels, int width, int height) {
    unsigned char* data = (unsigned char*)malloc(width * height * 3);
    for (int i = 0; i < width * height; i++) {
        data[i * 3 + 0] = (unsigned char)(255.99 * fmin(1.0, fmax(0.0, pixels[i].x)));
        data[i * 3 + 1] = (unsigned char)(255.99 * fmin(1.0, fmax(0.0, pixels[i].y)));
        data[i * 3 + 2] = (unsigned char)(255.99 * fmin(1.0, fmax(0.0, pixels[i].z)));
    }
    stbi_write_png(filename, width, height, 3, data, width * 3);
    free(data);
}

int main(int argc, char* argv[]) {
    OutputFormat format = FORMAT_PPM;
    const char* output_file = "output.ppm";

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--format") == 0 && i + 1 < argc) {
            if (strcmp(argv[i + 1], "png") == 0) {
                format = FORMAT_PNG;
                output_file = "output.png";
            }
            i++;
        }
    }

    FILE* fp = NULL;
    Vector3* pixels = NULL;

    if (format == FORMAT_PPM) {
        fp = fopen(output_file, "w");
        if (!fp) {
            fprintf(stderr, "Error: Could not open output file\n");
            return 1;
        }
        fprintf(fp, "P3\n%d %d\n255\n", WIDTH, HEIGHT);
    } else {
        pixels = (Vector3*)malloc(WIDTH * HEIGHT * sizeof(Vector3));
        if (!pixels) {
            fprintf(stderr, "Error: Could not allocate memory for pixels\n");
            return 1;
        }
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
            
            if (format == FORMAT_PPM) {
                write_color_ppm(fp, color);
            } else {
                pixels[(HEIGHT - 1 - j) * WIDTH + i] = color;
            }
        }
    }

    fprintf(stderr, "\nDone.\n");

    if (format == FORMAT_PPM) {
        fclose(fp);
    } else {
        save_png(output_file, pixels, WIDTH, HEIGHT);
        free(pixels);
    }

    return 0;
}
