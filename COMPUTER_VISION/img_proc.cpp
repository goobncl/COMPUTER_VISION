#include "stdafx.h"
#include "img_proc.h"
#include <omp.h>


void invertColors(unsigned char* input, int width, int height) {

	for (int i = 0; i < width * height; ++i) {
		input[i] = 255 - input[i];
	}
}

void histogramEqualization(unsigned char* input, int width, int height) {
    
    int size = width * height;
    int num_bins = 256;

    int histogram[256] = { 0 };
    for (int i = 0; i < size; ++i) {
        histogram[input[i]]++;
    }

    int cdf[256] = { 0 };
    int cdf_min = INT_MAX;
    int cdf_max = INT_MIN;

    cdf[0] = histogram[0];
    cdf_min = cdf[0];
    cdf_max = cdf[0];

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
