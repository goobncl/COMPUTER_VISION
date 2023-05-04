#include "stdafx.h"
#include "CLAHE.h"

void computeHistogram(unsigned char* input, int width, int subWidth, int subHeight, int row, int col, int histogram[256]) {

    for (int i = row * subHeight; i < (row + 1) * subHeight; ++i) {
        for (int j = col * subWidth; j < (col + 1) * subWidth; ++j) {
            int pixel_value = input[i * width + j];
            histogram[pixel_value]++;
        }
    }
}

void redistributeExcess(int histogram[256], int num_bins, int clipValue) {

    int excess = 0;
    for (int i = 0; i < num_bins; ++i) {
        int excess_value = histogram[i] - clipValue;
        if (excess_value > 0) {
            excess += excess_value;
            histogram[i] = clipValue;
        }
    }

    int redistribute = excess / num_bins;
    for (int i = 0; i < num_bins; ++i) {
        histogram[i] += redistribute;
    }
}

void computeCDF(int histogram[256], int cdf[256], int num_bins, int* cdf_min, int* cdf_max) {

    cdf[0] = histogram[0];
    *cdf_min = cdf[0];
    *cdf_max = cdf[0];
    for (int i = 1; i < num_bins; ++i) {
        cdf[i] = cdf[i - 1] + histogram[i];
        if (cdf[i] < *cdf_min) {
            *cdf_min = cdf[i];
        }
        if (cdf[i] > *cdf_max) {
            *cdf_max = cdf[i];
        }
    }
}

void applyMapping(unsigned char* input, int width, int subWidth, int subHeight, int row, int col, int cdf[256], int cdf_min, int cdf_range) {

    for (int i = row * subHeight; i < (row + 1) * subHeight; ++i) {
        for (int j = col * subWidth; j < (col + 1) * subWidth; ++j) {
            input[i * width + j] = (unsigned char)(((cdf[input[i * width + j]] - cdf_min) * 255) / cdf_range);
        }
    }
}

void gridHistogramEqualization(unsigned char* input, int width, int height) {

    int size = width * height;
    int num_bins = 256;
    int subWidth = width / 8;
    int subHeight = height / 6;
    double alpha = 0.0001;
    int mapping_functions[8][6][256] = { 0 };

    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 8; ++col) {

            int histogram[256] = { 0 };
            int Nb = subWidth * subHeight;
            int L = num_bins;
            int clipValue = (int)((Nb / L) + alpha * (Nb - Nb / L));
            int cdf[256] = { 0 };
            int cdf_min, cdf_max;

            computeHistogram(input, width, subWidth, subHeight, row, col, histogram);
            redistributeExcess(histogram, num_bins, clipValue);
            computeCDF(histogram, cdf, num_bins, &cdf_min, &cdf_max);

            int cdf_range = cdf_max - cdf_min;
            applyMapping(input, width, subWidth, subHeight, row, col, cdf, cdf_min, cdf_range);

            for (int i = 0; i < num_bins; ++i) {
                mapping_functions[row][col][i] = ((cdf[i] - cdf_min) * 255) / cdf_range;
            }
        }
    }
}