#pragma once

#ifndef CLAHE_H
#define CLAHE_H

#include <QDebug>

#define GRAY_LVL 256
#define SUB_W 8
#define SUB_H 6
#define ALPHA 0.0001
#define OVERLAP 10


extern int mapping_functions[SUB_H][SUB_W][GRAY_LVL];

void computeHistogram(unsigned char* input, int width, int subWidth, int subHeight, int row, int col, int histogram[GRAY_LVL]);
void redistributeExcess(int histogram[GRAY_LVL], int num_bins, int clipValue);
void computeCDF(int histogram[GRAY_LVL], int cdf[GRAY_LVL], int num_bins, int* cdf_min, int* cdf_max);
void applyMapping(unsigned char* input, int width, int subWidth, int subHeight, int row, int col, int cdf[GRAY_LVL], int cdf_min, int cdf_range);
void bilinearInterpolation(unsigned char* input, int width, int height, int subWidth, int subHeight, int mapping_functions[SUB_H][SUB_W][GRAY_LVL]);
void gridHistogramEqualization(unsigned char* input, int width, int height);

#endif