#include "stdafx.h"
#include "CLAHE.h"

#define GRAY_LVL 256
#define SUB_W 8
#define SUB_H 6
#define ALPHA 0.0001


void computeHistogram(unsigned char* input, int width, int subWidth, int subHeight, int row, int col, int histogram[GRAY_LVL]) {

    for (int i = row * subHeight; i < (row + 1) * subHeight; ++i) {
        for (int j = col * subWidth; j < (col + 1) * subWidth; ++j) {
            int pixel_value = input[i * width + j];
            histogram[pixel_value]++;
        }
    }
}

void redistributeExcess(int histogram[GRAY_LVL], int clipValue) {

    int excess = 0;
    for (int i = 0; i < GRAY_LVL; ++i) {
        int excess_value = histogram[i] - clipValue;
        if (excess_value > 0) {
            excess += excess_value;
            histogram[i] = clipValue;
        }
    }

    int redistribute = excess / GRAY_LVL;
    for (int i = 0; i < GRAY_LVL; ++i) {
        histogram[i] += redistribute;
    }
}

void computeCDF(int histogram[GRAY_LVL], int cdf[GRAY_LVL], int* cdf_min, int* cdf_max) {

    cdf[0] = histogram[0];
    *cdf_min = cdf[0];
    *cdf_max = cdf[0];
    for (int i = 1; i < GRAY_LVL; ++i) {
        cdf[i] = cdf[i - 1] + histogram[i];
        if (cdf[i] < *cdf_min) {
            *cdf_min = cdf[i];
        }
        if (cdf[i] > *cdf_max) {
            *cdf_max = cdf[i];
        }
    }
}

void applyMapping(unsigned char* input, int width, int subWidth, int subHeight, int row, int col, int cdf[GRAY_LVL], int cdf_min, int cdf_range) {

    for (int i = row * subHeight; i < (row + 1) * subHeight; ++i) {
        for (int j = col * subWidth; j < (col + 1) * subWidth; ++j) {
            input[i * width + j] = (unsigned char)(((cdf[input[i * width + j]] - cdf_min) * 255) / cdf_range);
        }
    }
}

void gridHistogramEqualization(unsigned char* input, int width, int height) {

    int size = width * height;
    int subWidth = width / SUB_W;
    int subHeight = height / SUB_H;
    int mapping_functions[SUB_W][SUB_H][GRAY_LVL] = { 0 };

    for (int row = 0; row < SUB_H; ++row) {
        for (int col = 0; col < SUB_W; ++col) {

            int histogram[GRAY_LVL] = { 0 };
            int Nb = subWidth * subHeight;
            int L = GRAY_LVL;
            int clipValue = (int)((Nb / L) + ALPHA * (Nb - Nb / L));
            int cdf[GRAY_LVL] = { 0 };
            int cdf_min, cdf_max;

            computeHistogram(input, width, subWidth, subHeight, row, col, histogram);
            redistributeExcess(histogram, clipValue);
            computeCDF(histogram, cdf, &cdf_min, &cdf_max);

            int cdf_range = cdf_max - cdf_min;
            applyMapping(input, width, subWidth, subHeight, row, col, cdf, cdf_min, cdf_range);

            for (int i = 0; i < GRAY_LVL; ++i) {
                mapping_functions[row][col][i] = ((cdf[i] - cdf_min) * 255) / cdf_range;
            }
        }
    }
}