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

            for (int i = row * subHeight; i < (row + 1) * subHeight; ++i) {
                for (int j = col * subWidth; j < (col + 1) * subWidth; ++j) {
                    int pixel_value = input[i * width + j];
                    histogram[pixel_value]++;
                }
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

            cdf[0] = histogram[0];
            int cdf_min = cdf[0];
            int cdf_max = cdf[0];
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
            for (int i = row * subHeight; i < (row + 1) * subHeight; ++i) {
                for (int j = col * subWidth; j < (col + 1) * subWidth; ++j) {
                    input[i * width + j] = (unsigned char)(((cdf[input[i * width + j]] - cdf_min) * 255) / cdf_range);
                }
            }

            for (int i = 0; i < num_bins; ++i) {
                mapping_functions[row][col][i] = ((cdf[i] - cdf_min) * 255) / cdf_range;
            }
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            
            int row = y / subHeight;
            int col = x / subWidth;
            
            int O1_x = col * subWidth;
            int O1_y = row * subHeight;
            int O2_x = (col + 1) * subWidth;
            int O2_y = row * subHeight;
            int O3_x = col * subWidth;
            int O3_y = (row + 1) * subHeight;
            int O4_x = (col + 1) * subWidth;
            int O4_y = (row + 1) * subHeight;
            
            int dx1 = x - O1_x;
            int dy1 = y - O1_y;
            double d1 = (double)(dx1 * dx1 + dy1 * dy1);
    
            int dx2 = x - O2_x;
            int dy2 = y - O2_y;
            double d2 = (double)(dx2 * dx2 + dy2 * dy2);
    
            int dx3 = x - O3_x;
            int dy3 = y - O3_y;
            double d3 = (double)(dx3 * dx3 + dy3 * dy3);
    
            int dx4 = x - O4_x;
            int dy4 = y - O4_y;
            double d4 = (double)(dx4 * dx4 + dy4 * dy4);
            
            int pixel_value = input[y * width + x];
            double T1 = mapping_functions[row][col][pixel_value];
            double T2 = mapping_functions[row][col + 1][pixel_value];
            double T3 = mapping_functions[row + 1][col][pixel_value];
            double T4 = mapping_functions[row + 1][col + 1][pixel_value];
            
            double Tp = (d3 * d4 * T1 + d1 * d4 * T2 + d1 * d2 * T3 + d2 * d3 * T4) / ((d1 + d3) * (d2 + d4));
            input[y * width + x] = (unsigned char)(Tp);
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
