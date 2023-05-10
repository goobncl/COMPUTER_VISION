#include "stdafx.h"
#include "proto_img_proc.h"


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
