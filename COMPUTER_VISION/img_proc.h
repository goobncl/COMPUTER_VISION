#pragma once

#ifndef IMG_PROC_H
#define IMG_PROC_H

#include <QDebug>


void invertColors(unsigned char* input, int width, int height);
void applyContrastAndNoise(unsigned char* input, int width, int height);
void histogramEqualization(unsigned char* input, int width, int height);
double calc_sqrt(double x);
void gridHistogramEqualization(unsigned char* input, int width, int height);
void prototype(unsigned char* input, int width, int height);

#endif // IMG_PROC_H