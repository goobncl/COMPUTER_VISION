#include "stdafx.h"
#include "proto_img_proc.h"


void invertColors(unsigned char* input, int width, int height) {

	for (int i = 0; i < width * height; ++i) {
		input[i] = 255 - input[i];
	}
}

void applyContrastAndNoise(unsigned char* input, int width, int height) {
    float contrast_factor = 1.5f;
    for (int i = 0; i < width * height; ++i) {
        input[i] = (unsigned char)((input[i] * contrast_factor) > 255 ? 255 : (input[i] * contrast_factor) < 0 ? 0 : (input[i] * contrast_factor));
    }

    srand((unsigned)time(NULL));
    int noise_range = 10;

    for (int i = 0; i < width * height; ++i) {
        int noise = (rand() % (2 * noise_range + 1)) - noise_range;
        input[i] = (unsigned char)((input[i] + noise) > 255 ? 255 : (input[i] + noise) < 0 ? 0 : (input[i] + noise));
    }
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

    unsigned char* output = (unsigned char *)malloc(width * height * sizeof(unsigned char));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int outputPixel = 0;
            for (int ky = -KERNEL_HALF; ky <= KERNEL_HALF; ++ky) {
                for (int kx = -KERNEL_HALF; kx <= KERNEL_HALF; ++kx) {
                    int iy = y + ky;
                    int ix = x + kx;
                    if (iy >= 0 && iy < height && ix >= 0 && ix < width) {
                        outputPixel += input[iy * width + ix] * kernel[ky + KERNEL_HALF][kx + KERNEL_HALF];
                    }
                }
            }
            output[y * width + x] = outputPixel / kernelSum;
        }
    }

    memcpy(input, output, width * height * sizeof(unsigned char));
    free(output);
}
