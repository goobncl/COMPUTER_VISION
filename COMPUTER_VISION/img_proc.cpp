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

    // Calculate the histogram
    int histogram[256] = { 0 };
    for (int i = 0; i < size; ++i) {
        histogram[input[i]]++;
    }

    // Calculate the cumulative distribution function (CDF)
    int cdf[256] = { 0 };
    cdf[0] = histogram[0];
    for (int i = 1; i < num_bins; ++i) {
        cdf[i] = cdf[i - 1] + histogram[i];
    }

    // Find the minimum and maximum values in the CDF
    int cdf_min = INT_MAX;
    int cdf_max = INT_MIN;
    for (int i = 0; i < num_bins; ++i) {
        if (cdf[i] < cdf_min) {
            cdf_min = cdf[i];
        }
        if (cdf[i] > cdf_max) {
            cdf_max = cdf[i];
        }
    }

    // Normalize the CDF and apply the mapping to the input image
    int cdf_range = cdf_max - cdf_min;
    for (int i = 0; i < size; ++i) {
        input[i] = (unsigned char)(((cdf[input[i]] - cdf_min) * 255) / cdf_range);
    }
}
