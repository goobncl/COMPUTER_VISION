#include "stdafx.h"
#include "CLAHE.h"


int mapping_functions[SUB_H][SUB_W][GRAY_LVL] = { 0 };

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

void bilinearInterpolation(unsigned char* input, int width, int height, int subWidth, int subHeight, int mapping_functions[SUB_H][SUB_W][GRAY_LVL]) {

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            
            int row = i / subHeight;
            int col = j / subWidth;            
            double y_ratio = (double)(i % subHeight) / subHeight;
            double x_ratio = (double)(j % subWidth) / subWidth;

            y_ratio = y_ratio * y_ratio * (3 - 2 * y_ratio);
            x_ratio = x_ratio * x_ratio * (3 - 2 * x_ratio);

            int pixel_value = input[i * width + j];

            int T1 = mapping_functions[row][col][pixel_value];
            int T2 = col + 1 < SUB_W ? mapping_functions[row][col + 1][pixel_value] : T1;
            int T3 = row + 1 < SUB_H ? mapping_functions[row + 1][col][pixel_value] : T1;
            int T4 = (row + 1 < SUB_H && col + 1 < SUB_W) ? mapping_functions[row + 1][col + 1][pixel_value] : T1;

            double d1 = 1 - x_ratio;
            double d2 = x_ratio;
            double d3 = 1 - y_ratio;
            double d4 = y_ratio;

            int top = (int)(T1 * d1 * d3 + T2 * d2 * d3);
            int bottom = (int)(T3 * d1 * d4 + T4 * d2 * d4);

            input[i * width + j] = (unsigned char)(top + bottom);
        }
    }
}

void gridHistogramEqualization(unsigned char* input, int width, int height) {

    int size = width * height;
    int subWidth = width / SUB_W;
    int subHeight = height / SUB_H;
    
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

    bilinearInterpolation(input, width, height, subWidth, subHeight, mapping_functions);
}
