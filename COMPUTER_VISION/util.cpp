#include "stdafx.h"
#include "util.h"


size_t alignSize(size_t sz, int n) {
	return (sz + n - 1) & -n;
}

int intMin(int n1, int n2) {
	return n1 < n2 ? n1 : n2;
}

int intMax(int n1, int n2) {
    return n1 > n2 ? n1 : n2;
}

int intAbs(int n) {
    return n < 0 ? -n : n;
}

double doubleAbs(double n) {
	return n < 0 ? -n : n;
}

double doubleRound(double number) {
    return (number >= 0) ? (int)(number + 0.5) : (int)(number - 0.5);
}

void downSampling(unsigned char* src, unsigned char* dst, int new_width, int new_height) {

    const int original_width = 640;
    const int original_height = 480;

    double scale_x = (double)new_width / original_width;
    double scale_y = (double)new_height / original_height;

    for (int y = 0; y < new_height; y++) {
        for (int x = 0; x < new_width; x++) {

            double original_x = x / scale_x;
            double original_y = y / scale_y;

            if (original_x >= original_width - 1 || original_y >= original_height - 1) {
                dst[y * new_width + x] = 0;
                continue;
            }

            int original_x_int = (int)original_x;
            int original_y_int = (int)original_y;

            double fx = original_x - original_x_int;
            double fy = original_y - original_y_int;

            unsigned char p00 = src[original_y_int * original_width + original_x_int];
            unsigned char p01 = src[original_y_int * original_width + original_x_int + 1];
            unsigned char p10 = src[(original_y_int + 1) * original_width + original_x_int];
            unsigned char p11 = src[(original_y_int + 1) * original_width + original_x_int + 1];

            double w00 = (1 - fx) * (1 - fy);
            double w01 = fx * (1 - fy);
            double w10 = (1 - fx) * fy;
            double w11 = fx * fy;

            dst[y * new_width + x] = (unsigned char)(
                w00 * p00 + w01 * p01 + w10 * p10 + w11 * p11
            );
        }
    }
}

void integral(unsigned char* src, int* dst, int width, int height, int layer_offset) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int sum = src[y * width + x];
            if (y > 0) sum += dst[(y - 1) * width + x + layer_offset];
            if (x > 0) sum += dst[y * width + (x - 1) + layer_offset];
            if (y > 0 && x > 0) sum -= dst[(y - 1) * width + (x - 1) + layer_offset];
            dst[y * width + x + layer_offset] = sum;
        }
    }
}

void integralSquare(unsigned char* src, int* dst, int width, int height, int layer_offset) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int value = src[y * width + x];
            int square = value * value;
            if (y > 0) square += dst[(y - 1) * width + x + layer_offset];
            if (x > 0) square += dst[y * width + (x - 1) + layer_offset];
            if (y > 0 && x > 0) square -= dst[(y - 1) * width + (x - 1) + layer_offset];
            dst[y * width + x + layer_offset] = square;
        }
    }
}

int getPixelVal(int* src, int x, int y, int width) {
    if (x < 0 || y < 0) return 0;
    return src[y * width + x];
}

int calcAreaSum(int* src, int topLeftX, int topLeftY, int width) {

    int bottomRightX = topLeftX + 23;
    int bottomRightY = topLeftY + 23;

    int p0 = getPixelVal(src, bottomRightX, bottomRightY, width);
    int p1 = getPixelVal(src, bottomRightX, topLeftY - 1, width);
    int p2 = getPixelVal(src, topLeftX - 1, bottomRightY, width);
    int p3 = getPixelVal(src, topLeftX - 1, topLeftY - 1, width);

    return (p0 - p1 - p2 + p3);
}
