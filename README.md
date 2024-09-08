# Image Processing Application in C with Comparison Window

## Overview

This C-based application allows users to perform various image processing operations on PPM format images (`P3` or `P6`). Users can apply transformations such as converting the image to grayscale, generating a negative, applying an X-ray effect, rotating the image, or generating an aged effect. After applying these transformations, the application provides a side-by-side comparison of the original and modified images.

## Program Structure

The program is organized into several functions that handle different tasks:
- **Image Processing:** Functions that handle various image transformations.
- **Memory Management:** Functions to allocate and free memory for images.
- **File Operations:** Functions to load and save PPM images.
- **User Interface:** Functions to create and manage the main application window and the comparison window.

### 1. Header Includes

The program includes necessary headers for file operations, image processing, memory management, parallelization, and GUI operations.

```c
#include <windows.h>
#include <commdlg.h> 
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h> 
#include <io.h>
```

### 2. Constants

The program defines several constants, including image size limits, color values, and weights for grayscale conversion.

```c
#define MIN_IMAGE_SIZE 400
#define MAX_COLOR_VALUE 255
#define GRAYSCALE_RED_WEIGHT 0.299
#define GRAYSCALE_GREEN_WEIGHT 0.587
#define GRAYSCALE_BLUE_WEIGHT 0.114
```

### 3. Global Variables

The program uses global variables to store the image data, dimensions, and file path.

```c
char file_name[MAX_PATH];
int width, height;
Pixel **image = NULL;
Pixel **original_image = NULL;
```

### 4. Function Prototypes

The function prototypes declare the functions used for image processing, memory management, file operations, and UI management.

### 5. Main Function

The `WinMain()` function initializes the main application window, sets up the message loop, and displays the window to the user.

### 6. Window Procedures

#### Main Window Procedure

The `WindowProc()` function handles events in the main application window, including button clicks for loading images, applying transformations, and showing the comparison window.

#### Comparison Window Procedure

The `ComparisonWindowProc()` function handles the display and painting of the comparison window, where the original and modified images are shown side-by-side.

### 7. Function Implementations

#### Image Processing Functions

1. **Grayscale Conversion:** 
   - `convert_to_grayscale()` converts the image to grayscale by applying weighted sums to the RGB components.

2. **Negative Image:** 
   - `generate_negative_image()` creates a negative version of the image by inverting the RGB values.

3. **X-Ray Effect:** 
   - `generate_xray_image()` converts the image to grayscale and applies a power transformation for an X-ray effect.

4. **Image Rotation:** 
   - `rotate_image()` rotates the image by 90 degrees clockwise.

5. **Aged Effect:** 
   - `generate_aged_image()` simulates an aged or sepia-toned effect by adjusting the color intensity.

#### Memory Management Functions

- `allocate_image()` allocates memory for the image data.
- `free_image()` deallocates the memory used by the image.

#### File Operations Functions

- `load_image()` loads a PPM image from a file into memory.
- `save_image()` saves the processed image back to a file.

#### User Interface Functions

- `show_comparison_window()` creates and displays a window showing the original and modified images side-by-side.

### 8. Example Usage

After compiling and running the program, the user is presented with a window containing buttons to load an image, apply transformations, and view a comparison of the original and modified images.

### 9. Handling Large Images

The program includes logic to scale down large images to fit within the comparison window, ensuring both images are displayed side-by-side without exceeding the screen size.

### 10. Compiling and Running the Program

To compile the program:

```bash
gcc -o image_processing image_processing.c -fopenmp -lgdi32
```

To run the program:

```bash
./image_processing
```

### 11. Future Enhancements

- **Additional Image Formats:** Expand support to other formats such as PNG and JPEG.
- **Advanced Effects:** Implement more complex image processing techniques.
- **User Interface:** Develop a more sophisticated GUI for enhanced user interaction.
