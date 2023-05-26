#include "stdafx.h"
#include "util.h"


size_t alignSize(size_t sz, int n) {
	return (sz + n - 1) & -n;
}

double doubleAbs(double n) {
	return n < 0 ? -n : n;
}

double doubleRound(double number) {
    return (number >= 0) ? (int)(number + 0.5) : (int)(number - 0.5);
}

unsigned char* downSampling(unsigned char* input, int new_width, int new_height) {

    const int original_width = 640;
    const int original_height = 480;

    if (input == nullptr || new_width <= 0 || new_height <= 0) {
        return nullptr;
    }

    unsigned char* output = (unsigned char*)malloc(sizeof(unsigned char) * new_width * new_height);

    double scale_x = (double)new_width / original_width;
    double scale_y = (double)new_height / original_height;

    for (int y = 0; y < new_height; y++) {
        for (int x = 0; x < new_width; x++) {

            double original_x = x / scale_x;
            double original_y = y / scale_y;

            if (original_x >= original_width - 1 || original_y >= original_height - 1) {
                output[y * new_width + x] = 0;
                continue;
            }

            int original_x_int = (int)original_x;
            int original_y_int = (int)original_y;

            double fx = original_x - original_x_int;
            double fy = original_y - original_y_int;

            unsigned char p00 = input[original_y_int * original_width + original_x_int];
            unsigned char p01 = input[original_y_int * original_width + original_x_int + 1];
            unsigned char p10 = input[(original_y_int + 1) * original_width + original_x_int];
            unsigned char p11 = input[(original_y_int + 1) * original_width + original_x_int + 1];

            double w00 = (1 - fx) * (1 - fy);
            double w01 = fx * (1 - fy);
            double w10 = (1 - fx) * fy;
            double w11 = fx * fy;

            output[y * new_width + x] = (unsigned char)(
                w00 * p00 + w01 * p01 + w10 * p10 + w11 * p11
            );
        }
    }

    return output;
}
