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
    int start_frame = 0;
    int end_frame = 0;  // 0 means render single frame
    double frame_rate = 30.0;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--format") == 0 && i + 1 < argc) {
            if (strcmp(argv[i + 1], "png") == 0) {
                format = FORMAT_PNG;
                output_file = "frame_%04d.png";
            }
            i++;
        }
        else if (strcmp(argv[i], "--start-frame") == 0 && i + 1 < argc) {
            start_frame = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--end-frame") == 0 && i + 1 < argc) {
            end_frame = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--fps") == 0 && i + 1 < argc) {
            frame_rate = atof(argv[i + 1]);
            i++;
        }
    }

    // Validate animation parameters
    if (end_frame > 0 && end_frame < start_frame) {
        fprintf(stderr, "Error: end_frame must be greater than start_frame\n");
        return 1;
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

    // Initialize scene with depth of field parameters
    Scene scene = scene_create();
    scene.aperture = 0.3;        // Further increased aperture size for more pronounced depth of field
    scene.focal_distance = 6.0;   // Adjusted focal distance to focus on middle sphere
    scene.animation_state = animation_state_create(frame_rate);
    scene.animation_state.current_frame = start_frame;

    // Create spheres with advanced material properties
    // Create a glass sphere with advanced optical properties
    Sphere glass_sphere = sphere_create(vector_create(0, 0, -6), 1.0, vector_create(0.9, 0.9, 0.9), 0.8, 1.5, 1.0);
    glass_sphere.glossiness = 1.0;      // Perfect reflection
    glass_sphere.roughness = 0.05;      // Slightly rough surface
    glass_sphere.metallic = 0.0;        // Non-metallic
    glass_sphere.dispersion = 0.04;     // Strong chromatic aberration
    
    // Create a metallic sphere with physically-based properties
    Sphere metal_sphere = sphere_create(vector_create(2, 0.5, -4), 0.7, vector_create(0.9, 0.8, 0.7), 0.9, 2.4, 0.8);
    metal_sphere.glossiness = 0.8;      // Slightly rough metal
    metal_sphere.roughness = 0.2;       // Polished metal surface
    metal_sphere.metallic = 1.0;        // Fully metallic
    metal_sphere.dispersion = 0.0;      // No dispersion for metals
    
    // Create a water sphere with realistic optical properties
    Sphere water_sphere = sphere_create(vector_create(-2, -0.5, -8), 1.2, vector_create(0.7, 0.8, 0.9), 0.7, 1.33, 0.9);
    water_sphere.glossiness = 0.6;      // Water-like glossiness
    water_sphere.roughness = 0.1;       // Slight surface perturbation
    water_sphere.metallic = 0.0;        // Non-metallic
    water_sphere.dispersion = 0.02;     // Slight water dispersion
    
    scene_add_sphere(&scene, glass_sphere);  // Glass sphere at focal plane
    scene_add_sphere(&scene, metal_sphere);  // Metal sphere in front
    scene_add_sphere(&scene, water_sphere);  // Water sphere behind
    scene_add_sphere(&scene, sphere_create(vector_create(0, -101, -5), 100.0, vector_create(0.5, 0.5, 0.5), 0.1, 1.0, 0.5)); // Ground plane

    // Add lights
    // Enhanced lighting setup for better shadows and reflections
    scene_add_light(&scene, area_light_create(vector_create(5, 5, -5), vector_create(1, 0.95, 0.8), 1.2, 2.0));  // Main warm light
    scene_add_light(&scene, area_light_create(vector_create(-5, 4, -3), vector_create(0.7, 0.8, 1.0), 0.8, 1.5));  // Fill cool light

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
            Vector3 color = vector_create(0, 0, 0);
            const int samples_per_pixel = 16;  // Increased samples for better quality

            // Anti-aliasing: Take multiple samples per pixel
            for (int s = 0; s < samples_per_pixel; s++) {
                double u = ((double)i + ((double)rand() / RAND_MAX)) / (WIDTH - 1);
                double v = ((double)j + ((double)rand() / RAND_MAX)) / (HEIGHT - 1);

                Vector3 direction = vector_subtract(
                    vector_add(
                        vector_add(lower_left_corner,
                            vector_multiply(horizontal, u)),
                        vector_multiply(vertical, v)
                    ),
                    camera_pos
                );

                Ray ray = ray_create(camera_pos, direction);
                color = vector_add(color, scene_trace(&scene, ray, MAX_DEPTH));
            }
            
            // Average the color samples
            color = vector_divide(color, samples_per_pixel);
            
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
