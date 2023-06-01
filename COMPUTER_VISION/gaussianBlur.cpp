#include "stdafx.h"
#include "util.h"
#include "gaussianBlur.h"


void gaussianBlur(unsigned char* input, int width, int height) {
    int kernel[KERNEL_SIZE] = { 1, 4, 6, 4, 1 };
    int kernelSum = 16;

    unsigned char* output = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    if (output == NULL) {
        return;
    }

    unsigned char* temp = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    if (temp == NULL) {
        free(output);
        return;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double outputPixel = 0.0;
            for (int kx = -KERNEL_HALF; kx <= KERNEL_HALF; ++kx) {
                int ix = x + kx;
                if (ix >= 0 && ix < width) {
                    outputPixel += input[y * width + ix] * kernel[kx + KERNEL_HALF];
                }
            }
            temp[y * width + x] = (unsigned char)doubleRound(outputPixel / kernelSum);
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double outputPixel = 0.0;
            for (int ky = -KERNEL_HALF; ky <= KERNEL_HALF; ++ky) {
                int iy = y + ky;
                if (iy >= 0 && iy < height) {
                    outputPixel += temp[iy * width + x] * kernel[ky + KERNEL_HALF];
                }
            }
            output[y * width + x] = (unsigned char)doubleRound(outputPixel / kernelSum);
        }
    }

    if (input != NULL && output != NULL) {
        memcpy(input, output, width * height * sizeof(unsigned char));
    }
    free(temp);
    free(output);
}
