#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "scene.h"
#include "scene_config.h"

#include "stb_image.h"
#include "stb_image_write.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

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

    // Load scene from configuration file or create default scene
    Scene* scene = NULL;
    const char* config_file = NULL;
    
    // Check for scene configuration file in arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--scene") == 0 && i + 1 < argc) {
            config_file = argv[i + 1];
            i++;
        }
    }
    
    if (config_file) {
        scene = load_scene_from_config(config_file);
        if (!scene) {
            fprintf(stderr, "Error: Failed to load scene from config file\n");
            return 1;
        }
    } else {
        scene = malloc(sizeof(Scene));
        *scene = scene_create();
        scene->aperture = 0.3;
        scene->focal_distance = 6.0;
    }
    
    scene->animation_state = animation_state_create(frame_rate);
    scene->animation_state.current_frame = start_frame;

    // If no config file was provided, set up default scene
    if (!config_file) {
        // Setup animation track for glass sphere
        AnimationTrack* glass_sphere_track = animation_track_create();
        // Create keyframes for circular motion
        for (int i = 0; i <= 60; i++) {
            double angle = (i / 60.0) * 2.0 * M_PI;  // Full circle over 60 frames
            double x = 2.0 * cos(angle);  // Radius of 2
            double z = -6.0 + 2.0 * sin(angle);  // Center at z = -6
            
            Keyframe keyframe = {
                .time = i / 30.0,  // 30 FPS
                .position = vector_create(x, 0, z),
                .rotation = vector_create(0, angle, 0),
                .scale = vector_create(1, 1, 1),
                .velocity = vector_create(-2.0 * sin(angle), 0, 2.0 * cos(angle))  // Tangential velocity
            };
            animation_track_add_keyframe(glass_sphere_track, keyframe);
        }

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
        
        scene_add_sphere(scene, glass_sphere);  // Glass sphere at focal plane
        scene->sphere_animations[0] = glass_sphere_track;  // Assign animation track to glass sphere
        scene->motion_blur_intensity = 0.5;  // Enable motion blur
        scene_add_sphere(scene, metal_sphere);  // Metal sphere in front
        scene_add_sphere(scene, water_sphere);  // Water sphere behind
        scene_add_sphere(scene, sphere_create(vector_create(0, -101, -5), 100.0, vector_create(0.5, 0.5, 0.5), 0.1, 1.0, 0.5)); // Ground plane

        // Add lights
        // Enhanced lighting setup for better shadows and reflections
        scene_add_light(scene, area_light_create(vector_create(5, 5, -5), vector_create(1, 0.95, 0.8), 1.2, 2.0));  // Main warm light
        scene_add_light(scene, area_light_create(vector_create(-5, 4, -3), vector_create(0.7, 0.8, 1.0), 0.8, 1.5));  // Fill cool light
    }

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

    // Animation rendering loop
    int total_frames = end_frame > 0 ? (end_frame - start_frame + 1) : 1;
    
    for (int frame = 0; frame < total_frames; frame++) {
        scene->animation_state.current_frame = start_frame + frame;
        scene->animation_state.current_time = scene->animation_state.current_frame / frame_rate;
        
        char frame_filename[256];
        if (format == FORMAT_PNG) {
            snprintf(frame_filename, sizeof(frame_filename), output_file, scene->animation_state.current_frame);
        }
        
        fprintf(stderr, "\nRendering frame %d/%d\n", frame + 1, total_frames);
        
        // Render scene
        for (int j = HEIGHT - 1; j >= 0; j--) {
            fprintf(stderr, "\rScanlines remaining: %d ", j);
            for (int i = 0; i < WIDTH; i++) {
                Vector3 color = vector_create(0, 0, 0);
                const int samples_per_pixel = 4;   // Reduced samples for better performance
                const int motion_samples = scene->motion_blur_intensity > 0 ? 4 : 1; // Reduced motion blur samples

            // Anti-aliasing and motion blur sampling
            for (int s = 0; s < samples_per_pixel; s++) {
                for (int m = 0; m < motion_samples; m++) {
                    // Calculate time offset for motion blur
                    double time_offset = 0.0;
                    if (motion_samples > 1) {
                        time_offset = ((double)m / (motion_samples - 1) - 0.5) * 
                                    scene->motion_blur_intensity * scene->animation_state.time_step;
                    }
                    
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
                    ray.time = scene->animation_state.current_time + time_offset;
                    color = vector_add(color, scene_trace(scene, ray, MAX_DEPTH));
                }
            }
            
            // Average the color samples (including motion blur samples)
            color = vector_divide(color, samples_per_pixel * motion_samples);
            
            if (format == FORMAT_PPM) {
                write_color_ppm(fp, color);
            } else {
                pixels[(HEIGHT - 1 - j) * WIDTH + i] = color;
            }
        }
    }

    fprintf(stderr, "\nDone.\n");

    // Save frame
        if (format == FORMAT_PPM) {
            // For PPM format, we only support single frame output
            fclose(fp);
            break;
        } else {
            save_png(frame_filename, pixels, WIDTH, HEIGHT);
        }
        
        // Update animation state
        animation_update_state(&scene->animation_state);
    }
    
    if (pixels) {
        free(pixels);
    }

    return 0;
}
