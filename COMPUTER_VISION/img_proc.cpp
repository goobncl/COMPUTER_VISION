#include "stdafx.h"
#include "img_proc.h"


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

void histogramEqualization(unsigned char* input, int width, int height) {

    int size = width * height;
    int num_bins = 256;
    int clipValue = 100;

    int histogram[256] = { 0 };
    for (int i = 0; i < size; ++i) {
        histogram[input[i]]++;
    }

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

    int cdf[256] = { 0 };
    int cdf_min = histogram[0];
    int cdf_max = histogram[0];
    //int cdf_min = INT_MAX;
    //int cdf_max = INT_MIN;

    cdf[0] = histogram[0];

    for (int i = 1; i < num_bins; ++i) {
        cdf[i] = cdf[i - 1] + histogram[i];
        if (cdf[i] < cdf_min) {
            cdf_min = cdf[i];
        }
        if (cdf[i] > cdf_max) {
            cdf_max = cdf[i];
        }
    }

    int cdf_range = cdf_max - cdf_min;
    for (int i = 0; i < size; ++i) {
        input[i] = (unsigned char)(((cdf[input[i]] - cdf_min) * 255) / cdf_range);
    }
}

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

void prototype(unsigned char* input, int width, int height) {
    
    int window_size = 8;
    int half_window = window_size / 2;

    int roi_width = 200;
    int roi_height = 150;

    int x_start = (width / 2) - (roi_width / 2);
    int x_end = (width / 2) + (roi_width / 2);
    int y_start = (height / 2) - (roi_height / 2);
    int y_end = (height / 2) + (roi_height / 2);

    unsigned char* temp_input = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    memcpy(temp_input, input, width * height * sizeof(unsigned char));

    for (int y = y_start; y < y_end; ++y) {
        for (int x = x_start; x < x_end; ++x) {
            int x_min = (x - half_window) >= 0 ? (x - half_window) : 0;
            int x_max = (x + half_window) < width ? (x + half_window) : (width - 1);
            int y_min = (y - half_window) >= 0 ? (y - half_window) : 0;
            int y_max = (y + half_window) < height ? (y + half_window) : (height - 1);

            int size = (x_max - x_min + 1) * (y_max - y_min + 1);
            int num_bins = 256;
            int histogram[256] = { 0 };

            for (int j = y_min; j <= y_max; ++j) {
                for (int i = x_min; i <= x_max; ++i) {
                    histogram[temp_input[j * width + i]]++;
                }
            }

            int cdf[256] = { 0 };
            cdf[0] = histogram[0];
            for (int i = 1; i < num_bins; ++i) {
                cdf[i] = cdf[i - 1] + histogram[i];
            }

            int cdf_min = cdf[0];
            int cdf_max = cdf[255];
            int cdf_range = cdf_max - cdf_min;

            input[y * width + x] = (unsigned char)(((cdf[temp_input[y * width + x]] - cdf_min) * 255) / cdf_range);
        }
    }

    free(temp_input);
}
