#pragma once

#ifndef UTIL_H
#define UTIL_H

size_t alignSize(size_t sz, int n);
int intMin(int n1, int n2);
int intMax(int n1, int n2);
int intAbs(int n);
double doubleAbs(double n);
double doubleRound(double number);
int doubleToInt(double n);
void downSampling(unsigned char* src, unsigned char* dst, int new_width, int new_height);
void integral(unsigned char* src, int* dst, int width, int height, int layer_offset);
void integralSquare(unsigned char* src, int* dst, int width, int height, int layer_offset);
int getPixelVal(int* src, int x, int y, int width);
int calcAreaSum(int* src, int topLeftX, int topLeftY, int width);

#endif // UTIL_H