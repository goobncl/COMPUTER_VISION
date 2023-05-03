#include "stdafx.h"
#include "img_proc.h"


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
