#pragma once

#ifndef CLAHE_H
#define CLAHE_H

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

#endif // CLAHE_H
