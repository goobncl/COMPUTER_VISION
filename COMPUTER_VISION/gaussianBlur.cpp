#include "stdafx.h"
#include "gaussianBlur.h"

double doubleRound(double number) {
    return (number >= 0) ? (int)(number + 0.5) : (int)(number - 0.5);
}

void gaussianBlur(unsigned char* input, int width, int height) {
    
    int kernel[KERNEL_SIZE][KERNEL_SIZE] = {
        { 1,  4,  6,  4, 1 },
        { 4, 16, 24, 16, 4 },
        { 6, 24, 36, 24, 6 },
        { 4, 16, 24, 16, 4 },
        { 1,  4,  6,  4, 1 }
    };
    int kernelSum = 256;

    unsigned char* output = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    if (output == NULL) {
        return;
    }
    memset(output, 0, width * height * sizeof(unsigned char));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double outputPixel = 0.0;
            for (int ky = -KERNEL_HALF; ky <= KERNEL_HALF; ++ky) {
                for (int kx = -KERNEL_HALF; kx <= KERNEL_HALF; ++kx) {
                    int iy = y + ky;
                    int ix = x + kx;
                    if (iy >= 0 && iy < height && ix >= 0 && ix < width) {
                        outputPixel += input[iy * width + ix] * kernel[ky + KERNEL_HALF][kx + KERNEL_HALF];
                    }
                }
            }
            output[y * width + x] = (unsigned char)doubleRound(outputPixel / kernelSum);
        }
    }

    if (input != NULL && output != NULL) {
        memcpy(input, output, width * height * sizeof(unsigned char));
    }
    free(output);
}
