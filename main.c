#include <windows.h>
#include <commdlg.h> // For common dialogs like file open
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h> // OpenMP for parallelization
#include <io.h> // For access function

// Constants
#define MIN_IMAGE_SIZE 400
#define MAX_COLOR_VALUE 255
#define GRAYSCALE_RED_WEIGHT 0.299
#define GRAYSCALE_GREEN_WEIGHT 0.587
#define GRAYSCALE_BLUE_WEIGHT 0.114

// Dimensions for the window and button sizes
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 500
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 40
#define BUTTON_MARGIN 10
const char* g_szClassName = "ImageProcessingWindow";
const char* g_welcomeClassName = "WelcomeWindow";
HINSTANCE g_hInstance;
HWND g_mainWindow;
void ShowMainWindow();
void ShowWelcomeWindow();

// Define maximum dimensions for the comparison window
#define MAX_WINDOW_WIDTH 1200
#define MAX_WINDOW_HEIGHT 800

// Structure to represent an RGB pixel
typedef struct {
    unsigned char r, g, b; // Red, Green, Blue components of a pixel
} Pixel;

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ComparisonWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
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
void process_image(HWND hwnd, int operation);
void apply_all_transformations(HWND hwnd);
void show_comparison_window(Pixel **original, Pixel **modified, int width, int height);


// Global variables
char file_name[MAX_PATH];
int width, height;
Pixel **image = NULL;
Pixel **original_image = NULL;

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "ImageProcessingWindow";
    WNDCLASS wc = {0};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Image Processing",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Calculate the initial position for the buttons
            int startX = (WINDOW_WIDTH - BUTTON_WIDTH) / 2; // Centered horizontally
            int startY = BUTTON_MARGIN;

            // Create the buttons
            CreateWindow("BUTTON", "Load Image", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         startX, startY, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU) 1, GetModuleHandle(NULL), NULL);

            startY += BUTTON_HEIGHT + BUTTON_MARGIN;
            CreateWindow("BUTTON", "Grayscale", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         startX, startY, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU) 2, GetModuleHandle(NULL), NULL);

            startY += BUTTON_HEIGHT + BUTTON_MARGIN;
            CreateWindow("BUTTON", "Negative", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         startX, startY, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU) 3, GetModuleHandle(NULL), NULL);

            startY += BUTTON_HEIGHT + BUTTON_MARGIN;
            CreateWindow("BUTTON", "X-ray", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         startX, startY, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU) 4, GetModuleHandle(NULL), NULL);

            startY += BUTTON_HEIGHT + BUTTON_MARGIN;
            CreateWindow("BUTTON", "Rotate", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         startX, startY, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU) 5, GetModuleHandle(NULL), NULL);

            startY += BUTTON_HEIGHT + BUTTON_MARGIN;
            CreateWindow("BUTTON", "Aged Effect", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         startX, startY, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU) 6, GetModuleHandle(NULL), NULL);

            startY += BUTTON_HEIGHT + BUTTON_MARGIN;
            CreateWindow("BUTTON", "Exit", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         startX, startY, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU) 8, GetModuleHandle(NULL), NULL);

            break;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case 1: {
                    OPENFILENAME ofn;
                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = file_name;
                    ofn.lpstrFile[0] = '\0';
                    ofn.nMaxFile = sizeof(file_name);
                    ofn.lpstrFilter = "PPM Files\0*.ppm\0All Files\0*.*\0";
                    ofn.nFilterIndex = 1;
                    ofn.lpstrFileTitle = NULL;
                    ofn.nMaxFileTitle = 0;
                    ofn.lpstrInitialDir = NULL;
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                    if (GetOpenFileName(&ofn)) {
                        image = load_image(file_name, &width, &height);
                        if (!image) {
                            MessageBox(hwnd, "Failed to load the image!", "Error", MB_OK | MB_ICONERROR);
                        } else {
                            MessageBox(hwnd, "Image loaded successfully!", "Success", MB_OK | MB_ICONINFORMATION);
                            // Save a copy of the original image
                            original_image = allocate_image(width, height);
                            memcpy(original_image[0], image[0], width * height * sizeof(Pixel));
                        }
                    }
                    break;
                }

                case 2: // Grayscale
                case 3: // Negative
                case 4: // X-ray
                case 5: // Rotate
                case 6: // Aged Effect
                    if (image) {
                        process_image(hwnd, wmId);
                        show_comparison_window(original_image, image, width, height);
                    } else {
                        MessageBox(hwnd, "No image loaded. Please load an image first.", "Error", MB_OK | MB_ICONERROR);
                    }
                    break;

                case 7: // Exit
                    PostQuitMessage(0);
                    break;

                default:
                    break;
            }
            break;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Function to handle image processing
void process_image(HWND hwnd, int operation) {
    // Ensure the "outputs" directory exists
    create_directory("outputs");

    switch (operation) {
        case 2: {
            convert_to_grayscale(image, width, height);
            save_image("grayscale_image.ppm", image, width, height);
            MessageBox(hwnd, "Grayscale transformation completed.", "Success", MB_OK | MB_ICONINFORMATION);
            break;
        }
        case 3: {
            generate_negative_image(image, width, height);
            save_image("negative_image.ppm", image, width, height);
            MessageBox(hwnd, "Negative transformation completed.", "Success", MB_OK | MB_ICONINFORMATION);
            break;
        }
        case 4: {
            generate_xray_image(image, width, height);
            save_image("xray_image.ppm", image, width, height);
            MessageBox(hwnd, "X-ray transformation completed.", "Success", MB_OK | MB_ICONINFORMATION);
            break;
        }
        case 5: {
            Pixel **rotated_image = rotate_image(image, width, height);
            if (rotated_image) {
                save_image("rotated_image.ppm", rotated_image, height, width);
                free_image(rotated_image);
                MessageBox(hwnd, "Rotation transformation completed.", "Success", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
        case 6: {
            generate_aged_image(image, width, height);
            save_image("aged_image.ppm", image, width, height);
            MessageBox(hwnd, "Aged effect applied successfully.", "Success", MB_OK | MB_ICONINFORMATION);
            break;
        }
        default:
            break;
    }
}

// Function to create a directory if it does not exist
void create_directory(const char *directory_name) {
    if (access(directory_name, 0) != 0) {  // Use access to check existence
        printf("Directory %s does not exist. Creating directory...\n", directory_name);
        if (mkdir(directory_name) != 0) {  // Use mkdir to create directory
            perror("Error creating directory");
        }
    } else {
        printf("Directory %s already exists.\n", directory_name);
    }
}

// Function to allocate memory for an image
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

// Function to free allocated memory for an image
void free_image(Pixel **image) {
    if (!image) {
        return;
    }
    free(image[0]);
    free(image);
}

// Function to load a PPM image from file
Pixel **load_image(const char *file_name, int *width, int *height) {
    char full_path[200];
    snprintf(full_path, sizeof(full_path), "%s", file_name);
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

// Function to save a PPM image to file
void save_image(const char *file_name, Pixel **image, int width, int height) {
    char full_path[200];
    snprintf(full_path, sizeof(full_path), "outputs/%s", file_name);

    printf("Trying to save the image to: %s\n", full_path);

    // Check if the directory exists
    if (access("outputs", 0) != 0) {
        printf("Directory 'outputs' does not exist or cannot be accessed.\n");
        create_directory("outputs");  // Ensure the "outputs" directory exists
        return;
    } else {
        printf("Directory 'outputs' exists and is accessible.\n");
    }

    FILE *file = fopen(full_path, "wb");
    if (!file) {
        printf("Error opening file %s for writing.\n", full_path);
        perror("fopen error"); // Print the specific error message
        return;
    }

    fprintf(file, "P6\n%d %d\n%d\n", width, height, MAX_COLOR_VALUE);
    for (int i = 0; i < height; i++) {
        fwrite(image[i], sizeof(Pixel), width, file);
    }

    fclose(file);
    printf("Image saved as %s\n", full_path);
}

// Function to convert the image to grayscale
 // Function to convert the image to grayscale
void convert_to_grayscale(Pixel **image, int width, int height) {
    printf("Converting image to grayscale...\n");

    // Apply parallel processing for the grayscale transformation
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // Calculate the grayscale value using the weighted sum method
            unsigned char gray = (unsigned char)(image[i][j].r * GRAYSCALE_RED_WEIGHT +
                                                image[i][j].g * GRAYSCALE_GREEN_WEIGHT +
                                                image[i][j].b * GRAYSCALE_BLUE_WEIGHT);
            // Set each color channel to the grayscale value
            image[i][j].r = image[i][j].g = image[i][j].b = gray;
        }
    }
    printf("Grayscale conversion completed.\n");
}

// Function to generate a negative of the image
void generate_negative_image(Pixel **image, int width, int height) {
    printf("Generating negative image...\n");

    // Use parallel processing to invert each pixel's color channels
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // Invert each color channel to achieve the negative effect
            image[i][j].r = MAX_COLOR_VALUE - image[i][j].r;
            image[i][j].g = MAX_COLOR_VALUE - image[i][j].g;
            image[i][j].b = MAX_COLOR_VALUE - image[i][j].b;
        }
    }
    printf("Negative image generated successfully.\n");
}

// Function to generate an X-ray effect on the image
void generate_xray_image(Pixel **image, int width, int height) {
    printf("Generating X-ray image...\n");

    // Start by converting the image to grayscale for the X-ray effect
    convert_to_grayscale(image, width, height);
    float factor = 1.5;

    // Apply parallel processing to enhance the grayscale image
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // Ensure grayscale value is non-zero to avoid potential math errors
            unsigned char gray = image[i][j].r;
            if (gray > 0) {
                gray = (unsigned char)pow(gray, factor);
            }
            // Clamp the value to the maximum color limit if it exceeds it
            image[i][j].r = image[i][j].g = image[i][j].b = (gray > MAX_COLOR_VALUE) ? MAX_COLOR_VALUE : gray;
        }
    }
    printf("X-ray image generated successfully.\n");
}

// Function to rotate the image by 90 degrees
Pixel **rotate_image(Pixel **image, int width, int height) {
    printf("Rotating the image by 90 degrees...\n");

    // Allocate memory for the rotated image with swapped dimensions
    Pixel **rotated_image = allocate_image(height, width); // Swap width and height for 90-degree rotation
    if (!rotated_image) {
        return NULL; // Return NULL if memory allocation fails
    }

    // Rotate the image by copying pixels to new positions
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // Rotate each pixel by 90 degrees counterclockwise
            rotated_image[j][height - i - 1] = image[i][j];
        }
    }

    printf("Rotation completed.\n");

    // Free original image memory if it won't be reused
    free_image(image);
    return rotated_image;
}

// Function to generate an aged effect on the image
void generate_aged_image(Pixel **image, int width, int height) {
    printf("Generating aged image...\n");
    float factor = 0.1; // Factor to adjust intensities for aging effect

    // Apply parallel processing to adjust each pixel for aging effect
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // Calculate new intensities with weighted adjustments to mimic aging
            float red_intensity = image[i][j].r * (1 + factor * (MAX_COLOR_VALUE - image[i][j].r) / (float)MAX_COLOR_VALUE);
            float green_intensity = image[i][j].g * (1 + factor * (MAX_COLOR_VALUE - image[i][j].g) / (float)MAX_COLOR_VALUE);
            float blue_intensity = image[i][j].b * (1 - factor * image[i][j].b / (float)MAX_COLOR_VALUE);

            // Further refine intensities to add an aged look
            red_intensity += 10 * (1 - image[i][j].r / (float)MAX_COLOR_VALUE);
            green_intensity += 10 * (1 - image[i][j].g / (float)MAX_COLOR_VALUE);
            blue_intensity -= 10 * (image[i][j].b / (float)MAX_COLOR_VALUE);

            // Clamp intensities to the allowed color range and set new values
            image[i][j].r = (unsigned char)fmin(fmax(red_intensity, 0), MAX_COLOR_VALUE);
            image[i][j].g = (unsigned char)fmin(fmax(green_intensity, 0), MAX_COLOR_VALUE);
            image[i][j].b = (unsigned char)fmin(fmax(blue_intensity, 0), MAX_COLOR_VALUE);
        }
    }

    printf("Aged image generated successfully.\n");
}


// Function to display a window comparing the original and modified images
void show_comparison_window(Pixel **original, Pixel **modified, int width, int height) {
    const char COMP_CLASS_NAME[] = "ComparisonWindow";
    WNDCLASS wc = {0};

    wc.lpfnWndProc = ComparisonWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = COMP_CLASS_NAME;

    RegisterClass(&wc);

    // Calculate the window size to fit both images
    int window_width = min(MAX_WINDOW_WIDTH, width * 2 + 20); // 20 pixels for margin
    int window_height = min(MAX_WINDOW_HEIGHT, height);

    HWND hwnd = CreateWindowEx(
        0,
        COMP_CLASS_NAME,
        "Comparison: Original vs Modified",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height + 50,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)modified); // Store the modified image data for later use

    ShowWindow(hwnd, SW_SHOW);
}
// Window procedure for the comparison window
LRESULT CALLBACK ComparisonWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Retrieve the modified image data from the window's user data
            Pixel **modified = (Pixel **)GetWindowLongPtr(hwnd, GWLP_USERDATA);

            // Calculate the original width and height
            int original_width = width;
            int original_height = height;

            // Determine the scaling factor to fit both images within the comparison window
            double scale_x = (double)(MAX_WINDOW_WIDTH / 2) / original_width;
            double scale_y = (double)(MAX_WINDOW_HEIGHT) / original_height;
            double scale = min(scale_x, scale_y);

            int scaled_width = (int)(original_width * scale);
            int scaled_height = (int)(original_height * scale);

            // Draw original and modified images side by side
            for (int y = 0; y < scaled_height; y++) {
                for (int x = 0; x < scaled_width; x++) {
                    int src_x = (int)(x / scale);
                    int src_y = (int)(y / scale);

                    // Draw original image on the left
                    SetPixel(hdc, x, y, RGB(original_image[src_y][src_x].r, original_image[src_y][src_x].g, original_image[src_y][src_x].b));

                    // Draw modified image on the right
                    SetPixel(hdc, x + scaled_width + 10, y, RGB(modified[src_y][src_x].r, modified[src_y][src_x].g, modified[src_y][src_x].b));
                }
            }

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;

        case WM_DESTROY:
            PostQuitMessage(0);
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

