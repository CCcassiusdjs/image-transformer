# Image Processing Program in C

## Overview

This program is a C-based image processing application that applies various transformations to images in PPM format (`P3` or `P6`). The user can choose from multiple options to transform the image, such as converting it to grayscale, generating a negative, applying an X-ray effect, rotating the image, or generating an aged effect. The program also allows the user to perform all transformations at once or to switch between different images.

## Program Structure

The program consists of several functions, each designed to handle different aspects of image processing. Here's an overview of the code structure:

1. **Header Includes**
2. **Constants**
3. **Structures**
4. **Function Prototypes**
5. **Main Function**
6. **Function Implementations**
   - Directory Management
   - Memory Management
   - Image Loading and Saving
   - Image Transformation Functions
   - User Interface (Menu)

## Detailed Breakdown

### 1. Header Includes

The program includes several headers necessary for file operations, image processing, memory management, and parallelization.

```c
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h> // OpenMP for parallelization
#include <io.h>  // For _access in Windows
#include <direct.h> // For _mkdir in Windows
```

### 2. Constants

The program defines constants used throughout the code, such as image size limits, color values, and weights for grayscale conversion.

```c
#define MIN_IMAGE_SIZE 400
#define MAX_COLOR_VALUE 255
#define GRAYSCALE_RED_WEIGHT 0.299
#define GRAYSCALE_GREEN_WEIGHT 0.587
#define GRAYSCALE_BLUE_WEIGHT 0.114
```

### 3. Structures

The `Pixel` structure is used to represent the RGB components of a pixel.

```c
typedef struct {
    unsigned char r, g, b; // Red, Green, Blue components of a pixel
} Pixel;
```

### 4. Function Prototypes

The program declares all the function prototypes, which include functions for:
- Directory management
- Memory allocation and deallocation
- Image loading and saving
- Image processing transformations
- User interface

### 5. Main Function

The `main()` function initializes the program, creates necessary directories, loads the image, and repeatedly displays a menu for user interaction. It handles user inputs for various image transformations.

### 6. Function Implementations

#### Directory Management

The `create_directory()` function checks if the directory exists and creates it if it does not.

```c
void create_directory(const char *directory_name) {
    if (_access(directory_name, 0) != 0) {
        printf("Directory %s does not exist. Creating directory...\n", directory_name);
        _mkdir(directory_name); // Create the directory
    } else {
        printf("Directory %s already exists.\n", directory_name);
    }
}
```

#### Memory Management

- `allocate_image()` allocates memory for an image.
- `free_image()` frees the allocated memory.

#### Image Loading and Saving

- `load_image()` reads a PPM file and loads the image data into memory.
- `save_image()` saves an image in PPM format to the specified directory.

#### Image Transformation Functions

1. **Convert to Grayscale**
    - The `convert_to_grayscale()` function converts the image to grayscale using the weighted sum of RGB components.

2. **Generate Negative Image**
    - The `generate_negative_image()` function inverts the color of each pixel to create a negative effect.

3. **Generate X-Ray Image**
    - The `generate_xray_image()` function converts the image to grayscale and applies a power transformation to enhance the details, creating an X-ray-like effect.

4. **Rotate Image**
    - The `rotate_image()` function rotates the image by 90 degrees.

5. **Generate Aged Image**
    - The `generate_aged_image()` function applies color adjustments to give the image an aged or sepia-toned appearance.

#### User Interface (Menu)

The `menu()` function displays the list of available options for the user.

### 7. Example Usage

After compiling and running the program, the user is prompted to enter the image filename. The program then provides the following menu:

```
Choose an option:
1. Convert to grayscale
2. Generate negative image
3. Generate X-ray image
4. Rotate image 90 degrees
5. Generate aged image
6. Apply all transformations (Todos)
7. Choose another image
0. Exit
```

The user selects an option, and the program applies the corresponding transformation(s) to the image.

## Additional Notes

- The program uses OpenMP for parallelizing the image processing operations to improve performance.
- Only PPM format images (`P3` and `P6`) are supported.
- The program ensures that the loaded images have a minimum size of 400x400 pixels to avoid processing extremely small images.

## Compiling and Running the Program

To compile the program, use the following command in a terminal or command prompt:

```bash
gcc -o image_processing image_processing.c -fopenmp
```

To run the program, execute:

```bash
./image_processing
```

## Future Improvements

- Support for additional image formats (e.g., PNG, JPEG).
- Implementation of more advanced image processing techniques (e.g., filters, edge detection).
- GUI interface for more user-friendly interaction.

---
