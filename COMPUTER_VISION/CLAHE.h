#pragma once

#ifndef CLAHE_H
#define CLAHE_H

#include <QDebug>

void computeHistogram(unsigned char* input, int width, int subWidth, int subHeight, int row, int col, int histogram[256]);
void redistributeExcess(int histogram[256], int num_bins, int clipValue);
void computeCDF(int histogram[256], int cdf[256], int num_bins, int* cdf_min, int* cdf_max);
void applyMapping(unsigned char* input, int width, int subWidth, int subHeight, int row, int col, int cdf[256], int cdf_min, int cdf_range);
void gridHistogramEqualization(unsigned char* input, int width, int height);

#endif