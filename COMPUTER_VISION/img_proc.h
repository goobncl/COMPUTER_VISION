#pragma once

#ifndef IMG_PROC_H
#define IMG_PROC_H

#include <QDebug>


void invertColors(unsigned char* input, int width, int height);
void applyContrastAndNoise(unsigned char* input, int width, int height);
void histogramEqualization(unsigned char* input, int width, int height);

void computeHistogram(unsigned char* input, int width, int subWidth, int subHeight, int row, int col, int histogram[256]);
void redistributeExcess(int histogram[256], int num_bins, int clipValue);
void computeCDF(int histogram[256], int cdf[256], int num_bins, int* cdf_min, int* cdf_max);
void applyMapping(unsigned char* input, int width, int subWidth, int subHeight, int row, int col, int cdf[256], int cdf_min, int cdf_range);
void gridHistogramEqualization(unsigned char* input, int width, int height);

void prototype(unsigned char* input, int width, int height);

#endif // IMG_PROC_H
