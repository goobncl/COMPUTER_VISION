#pragma once

#ifndef GAUSSIAN_BLUR_H
#define GAUSSIAN_BLUR_H

#define KERNEL_SIZE 5
#define KERNEL_HALF KERNEL_SIZE/2

double doubleRound(double number);
void gaussianBlur(unsigned char* input, int width, int height);

#endif // GAUSSIAN_BLUR_H
