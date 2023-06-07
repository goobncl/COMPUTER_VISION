#pragma once

#ifndef UTIL_H
#define UTIL_H

size_t alignSize(size_t sz, int n);
double doubleAbs(double n);
double doubleRound(double number);
void downSampling(unsigned char* src, unsigned char* dst, int new_width, int new_height);
void integral(unsigned char* src, int* dst, int width, int height, int layer_offset);
void integralSquare(unsigned char* src, int* dst, int width, int height, int layer_offset);

#endif // UTIL_H