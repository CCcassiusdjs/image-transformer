#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h> // OpenMP for parallelization
#include <io.h> // For _access in Windows
#include <direct.h> // For _mkdir in Windows

// Constants
#define MIN_IMAGE_SIZE 400
#define MAX_COLOR_VALUE 255
#define GRAYSCALE_RED_WEIGHT 0.299
#define GRAYSCALE_GREEN_WEIGHT 0.587
#define GRAYSCALE_BLUE_WEIGHT 0.114

// Structure to represent an RGB pixel
typedef struct {
    unsigned char r, g, b; // Red, Green, Blue components of a pixel
} Pixel;

// Function prototypes
void create_directory(const char *directory_name);
Pixel **allocate_image(int width, int height);
void free_image(Pixel **image);
Pixel **load_image(const char *file_name, int *width, int *height);
void save_image(const char *file_name, Pixel **image, int width, int height);
void convert_to_grayscale(Pixel **image, int width, int height);
void generate_negative_image(Pixel **image, int width, int height);
void generate_xray_image(Pixel **image, int width, int height);
Pixel **rotate_image(Pixel **image, int width, int height);
void generate_aged_image(Pixel **image, int width, int height);
void menu();

// Function definitions

void create_directory(const char *directory_name) {
    if (_access(directory_name, 0) != 0) {
        printf("Directory %s does not exist. Creating directory...\n", directory_name);
        _mkdir(directory_name); // Create the directory
    } else {
        printf("Directory %s already exists.\n", directory_name);
    }
}

Pixel **allocate_image(int width, int height) {
    Pixel **image = malloc(height * sizeof(Pixel *));
    if (!image) {
        printf("Memory allocation failed for image rows.\n");
        return NULL;
    }

    image[0] = (Pixel *)malloc(width * height * sizeof(Pixel));
    if (!image[0]) {
        printf("Memory allocation failed for image data.\n");
        free(image);
        return NULL;
    }

    for (int i = 1; i < height; i++) {
        image[i] = image[0] + i * width;
    }

    return image;
}

void free_image(Pixel **image) {
    if (!image) {
        return;
    }
    free(image[0]); // Free contiguous memory block
    free(image);    // Free pointer array
}

Pixel **load_image(const char *file_name, int *width, int *height) {
    char full_path[200];
    snprintf(full_path, sizeof(full_path), "../images/%s", file_name);
    printf("Trying to open the file: %s\n", full_path);

    FILE *file = fopen(full_path, "rb");
    if (!file) {
        printf("Error opening the file %s\n", full_path);
        return NULL;
    }

    printf("File %s opened successfully.\n", full_path);

    char format[3];
    fscanf(file, "%2s", format);
    if (strcmp(format, "P3") != 0 && strcmp(format, "P6") != 0) {
        printf("Invalid format. Only PPM images (P3 and P6) are supported.\n");
        fclose(file);
        return NULL;
    }

    printf("PPM format (%s) confirmed.\n", format);

    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == '#') {
            while (fgetc(file) != '\n' && !feof(file));
        } else if (!isspace(c)) {
            ungetc(c, file);
            break;
        }
    }

    if (fscanf(file, "%d %d", width, height) != 2) {
        printf("Error reading image dimensions.\n");
        fclose(file);
        return NULL;
    }

    int max_color;
    if (fscanf(file, "%d", &max_color) != 1) {
        printf("Error reading max color value.\n");
        fclose(file);
        return NULL;
    }
    fgetc(file);

    printf("Image loaded with dimensions: %d x %d and max color: %d\n", *width, *height, max_color);

    if (*width < MIN_IMAGE_SIZE || *height < MIN_IMAGE_SIZE) {
        printf("The image must be at least 400x400 pixels.\n");
        fclose(file);
        return NULL;
    }

    Pixel **image = allocate_image(*width, *height);
    if (!image) {
        fclose(file);
        return NULL;
    }

    if (strcmp(format, "P3") == 0) {
        for (int i = 0; i < *height; i++) {
            for (int j = 0; j < *width; j++) {
                int r, g, b;
                if (fscanf(file, "%d %d %d", &r, &g, &b) != 3) {
                    printf("Error reading pixel data.\n");
                    free_image(image);
                    fclose(file);
                    return NULL;
                }
                image[i][j].r = (unsigned char)(MAX_COLOR_VALUE * r / max_color);
                image[i][j].g = (unsigned char)(MAX_COLOR_VALUE * g / max_color);
                image[i][j].b = (unsigned char)(MAX_COLOR_VALUE * b / max_color);
            }
        }
    } else if (strcmp(format, "P6") == 0) {
        for (int i = 0; i < *height; i++) {
            fread(image[i], sizeof(Pixel), *width, file);
        }
    }

    fclose(file);
    printf("Image %s loaded successfully.\n", full_path);
    return image;
}

void save_image(const char *file_name, Pixel **image, int width, int height) {
    char full_path[200];
    snprintf(full_path, sizeof(full_path), "outputs/%s", file_name);

    printf("Trying to save the image to: %s\n", full_path);

    FILE *file = fopen(full_path, "wb");
    if (!file) {
        printf("Error saving the file %s\n", full_path);
        return;
    }

    fprintf(file, "P6\n%d %d\n%d\n", width, height, MAX_COLOR_VALUE);
    for (int i = 0; i < height; i++) {
        fwrite(image[i], sizeof(Pixel), width, file);
    }

    fclose(file);
    printf("Image saved as %s\n", full_path);
}

void convert_to_grayscale(Pixel **image, int width, int height) {
    printf("Converting image to grayscale...\n");

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            unsigned char gray = (unsigned char)(image[i][j].r * GRAYSCALE_RED_WEIGHT +
                                                image[i][j].g * GRAYSCALE_GREEN_WEIGHT +
                                                image[i][j].b * GRAYSCALE_BLUE_WEIGHT);
            image[i][j].r = image[i][j].g = image[i][j].b = gray;
        }
    }
    printf("Grayscale conversion completed.\n");
}

void generate_negative_image(Pixel **image, int width, int height) {
    printf("Generating negative image...\n");

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i][j].r = MAX_COLOR_VALUE - image[i][j].r;
            image[i][j].g = MAX_COLOR_VALUE - image[i][j].g;
            image[i][j].b = MAX_COLOR_VALUE - image[i][j].b;
        }
    }
    printf("Negative image generated successfully.\n");
}

void generate_xray_image(Pixel **image, int width, int height) {
    printf("Generating X-ray image...\n");

    convert_to_grayscale(image, width, height);
    float factor = 1.5;

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            unsigned char gray = (unsigned char)pow(image[i][j].r, factor);
            image[i][j].r = image[i][j].g = image[i][j].b = (gray > MAX_COLOR_VALUE) ? MAX_COLOR_VALUE : gray;
        }
    }
    printf("X-ray image generated successfully.\n");
}

Pixel **rotate_image(Pixel **image, int width, int height) {
    printf("Rotating the image by 90 degrees...\n");

    Pixel **rotated_image = allocate_image(height, width);
    if (!rotated_image) {
        return NULL;
    }

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            rotated_image[j][height - i - 1] = image[i][j];
        }
    }

    printf("Rotation completed.\n");
    return rotated_image;
}

void generate_aged_image(Pixel **image, int width, int height) {
    printf("Generating aged image...\n");
    float factor = 0.1;

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float red_intensity = image[i][j].r * (1 + factor * (MAX_COLOR_VALUE - image[i][j].r) / (float)MAX_COLOR_VALUE);
            float green_intensity = image[i][j].g * (1 + factor * (MAX_COLOR_VALUE - image[i][j].g) / (float)MAX_COLOR_VALUE);
            float blue_intensity = image[i][j].b * (1 - factor * image[i][j].b / (float)MAX_COLOR_VALUE);

            red_intensity += 10 * (1 - image[i][j].r / (float)MAX_COLOR_VALUE);
            green_intensity += 10 * (1 - image[i][j].g / (float)MAX_COLOR_VALUE);
            blue_intensity -= 10 * (image[i][j].b / (float)MAX_COLOR_VALUE);

            image[i][j].r = (unsigned char)fmin(fmax(red_intensity, 0), MAX_COLOR_VALUE);
            image[i][j].g = (unsigned char)fmin(fmax(green_intensity, 0), MAX_COLOR_VALUE);
            image[i][j].b = (unsigned char)fmin(fmax(blue_intensity, 0), MAX_COLOR_VALUE);
        }
    }

    printf("Aged image generated successfully.\n");
}

void menu() {
    printf("Choose an option:\n");
    printf("1. Convert to grayscale\n");
    printf("2. Generate negative image\n");
    printf("3. Generate X-ray image\n");
    printf("4. Rotate image 90 degrees\n");
    printf("5. Generate aged image\n");
    printf("6. Apply all transformations (Todos)\n");
    printf("7. Choose another image\n");
    printf("0. Exit\n");
}

int main() {
    create_directory("outputs");

    char file_name[100];
    printf("Enter the image name (format .ppm): ");
    scanf("%s", file_name);

    int width, height;
    Pixel **image = load_image(file_name, &width, &height);
    if (!image) {
        return 1;
    }

    while (1) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current working directory: %s\n", cwd);
        } else {
            perror("getcwd() error");
            free_image(image);
            return 1;
        }

        create_directory("outputs");

        menu();
        int option;
        scanf("%d", &option);

        switch (option) {
            case 1: {
                Pixel **image_copy = load_image(file_name, &width, &height);
                if (image_copy) {
                    convert_to_grayscale(image_copy, width, height);
                    save_image("grayscale_image.ppm", image_copy, width, height);
                    free_image(image_copy);
                }
                break;
            }
            case 2: {
                Pixel **image_copy = load_image(file_name, &width, &height);
                if (image_copy) {
                    generate_negative_image(image_copy, width, height);
                    save_image("negative_image.ppm", image_copy, width, height);
                    free_image(image_copy);
                }
                break;
            }
            case 3: {
                Pixel **image_copy = load_image(file_name, &width, &height);
                if (image_copy) {
                    generate_xray_image(image_copy, width, height);
                    save_image("xray_image.ppm", image_copy, width, height);
                    free_image(image_copy);
                }
                break;
            }
            case 4: {
                Pixel **image_copy = load_image(file_name, &width, &height);
                if (image_copy) {
                    Pixel **rotated_image = rotate_image(image_copy, width, height);
                    if (rotated_image) {
                        save_image("rotated_image.ppm", rotated_image, height, width);
                        free_image(rotated_image);
                    }
                    free_image(image_copy);
                }
                break;
            }
            case 5: {
                Pixel **image_copy = load_image(file_name, &width, &height);
                if (image_copy) {
                    generate_aged_image(image_copy, width, height);
                    save_image("aged_image.ppm", image_copy, width, height);
                    free_image(image_copy);
                }
                break;
            }
            case 6: {
                Pixel **image_copy = load_image(file_name, &width, &height);
                if (image_copy) {
                    convert_to_grayscale(image_copy, width, height);
                    save_image("grayscale_image.ppm", image_copy, width, height);
                    free_image(image_copy);
                }

                image_copy = load_image(file_name, &width, &height);
                if (image_copy) {
                    generate_negative_image(image_copy, width, height);
                    save_image("negative_image.ppm", image_copy, width, height);
                    free_image(image_copy);
                }

                image_copy = load_image(file_name, &width, &height);
                if (image_copy) {
                    generate_xray_image(image_copy, width, height);
                    save_image("xray_image.ppm", image_copy, width, height);
                    free_image(image_copy);
                }

                image_copy = load_image(file_name, &width, &height);
                if (image_copy) {
                    Pixel **rotated_image = rotate_image(image_copy, width, height);
                    if (rotated_image) {
                        save_image("rotated_image.ppm", rotated_image, height, width);
                        free_image(rotated_image);
                    }
                    free_image(image_copy);
                }

                image_copy = load_image(file_name, &width, &height);
                if (image_copy) {
                    generate_aged_image(image_copy, width, height);
                    save_image("aged_image.ppm", image_copy, width, height);
                    free_image(image_copy);
                }
                break;
            }
            case 7: {
                printf("Enter the image name (format .ppm): ");
                scanf("%s", file_name);

                free_image(image);
                image = load_image(file_name, &width, &height);
                if (!image) {
                    return 1;
                }
                break;
            }
            case 0:
                printf("Exiting the program.\n");
                free_image(image);
                return 0;
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}
