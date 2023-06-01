#pragma once

#ifndef UTIL_H
#define UTIL_H

size_t alignSize(size_t sz, int n);
double doubleAbs(double n);
double doubleRound(double number);
unsigned char* downSampling(unsigned char* input, int new_width, int new_height);
int* integral(unsigned char* input, int width, int height);
int* integralSquare(unsigned char* input, int width, int height);

#endif // UTIL_H