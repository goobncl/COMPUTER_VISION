#pragma once

#ifndef GITHUB_CLAHE_H
#define GITHUB_CLAHE_H

typedef unsigned char kz_pixel_t;
#define uiNR_OF_GREY (256)

int _CLAHE(	
	kz_pixel_t* pImage, 
	unsigned int uiXRes, 
	unsigned int uiYRes, 
	kz_pixel_t Min,
	kz_pixel_t Max, 
	unsigned int uiNrX, 
	unsigned int uiNrY,
	unsigned int uiNrBins, 
	float fCliplimit
);

int CLAHE(kz_pixel_t* input, unsigned int width, unsigned int height);

#endif // GITHUB_CLAHE_H
