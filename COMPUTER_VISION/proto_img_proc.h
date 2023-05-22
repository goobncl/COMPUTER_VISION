#pragma once

#ifndef IMG_PROC_H
#define IMG_PROC_H

#include <QDebug>

#define KERNEL_SIZE 5
#define KERNEL_HALF KERNEL_SIZE/2


void invertColors(unsigned char* input, int width, int height);
void applyContrastAndNoise(unsigned char* input, int width, int height);
void gaussianBlur(unsigned char* input, int width, int height);

#endif // IMG_PROC_H
