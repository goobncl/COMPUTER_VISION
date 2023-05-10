#include "stdafx.h"
#include "CLAHE.h"


static void ClipHistogram(unsigned long*, unsigned int, unsigned long);
static void MakeHistogram(kz_pixel_t*, unsigned int, unsigned int, unsigned int, unsigned long*, unsigned int, kz_pixel_t*);
static void MapHistogram(unsigned long*, kz_pixel_t, kz_pixel_t, unsigned int, unsigned long);
static void MakeLut(kz_pixel_t*, kz_pixel_t, kz_pixel_t, unsigned int);
static void Interpolate(kz_pixel_t*, int, unsigned long*, unsigned long*, unsigned long*, unsigned long*, unsigned int, unsigned int, kz_pixel_t*);

void ClipHistogram(
    unsigned long* pulHistogram, 
    unsigned int uiNrGreylevels, 
    unsigned long ulClipLimit)
{
    unsigned long* pulBinPointer;
    unsigned long* pulEndPointer;
    unsigned long* pulHisto;
    unsigned long ulNrExcess;
    unsigned long ulUpper; 
    unsigned long  ulBinIncr;
    unsigned long ulStepSize;
    unsigned long i;
    unsigned long ulOldNrExcess;
    long lBinExcess;

    ulNrExcess = 0;  
    pulBinPointer = pulHistogram;

    for (i = 0; i < uiNrGreylevels; i++) { 
        lBinExcess = (long)pulBinPointer[i] - (long)ulClipLimit;
        if (lBinExcess > 0) {
            ulNrExcess += lBinExcess;
        }
    }

    ulBinIncr = ulNrExcess / uiNrGreylevels;
    ulUpper = ulClipLimit - ulBinIncr;

    for (i = 0; i < uiNrGreylevels; i++) {
        if (pulHistogram[i] > ulClipLimit) {
            pulHistogram[i] = ulClipLimit;
        }
        else {
            if (pulHistogram[i] > ulUpper) {
                ulNrExcess -= pulHistogram[i] - ulUpper; pulHistogram[i] = ulClipLimit;
            }
            else {
                ulNrExcess -= ulBinIncr; pulHistogram[i] += ulBinIncr;
            }
        }
    }

    do {
        pulEndPointer = &pulHistogram[uiNrGreylevels]; 
        pulHisto = pulHistogram;
        ulOldNrExcess = ulNrExcess;

        while (ulNrExcess && pulHisto < pulEndPointer) {

            ulStepSize = uiNrGreylevels / ulNrExcess;
            if (ulStepSize < 1) {
                ulStepSize = 1;
            }
            for (pulBinPointer = pulHisto; pulBinPointer < pulEndPointer && ulNrExcess; pulBinPointer += ulStepSize) {
                if (*pulBinPointer < ulClipLimit) {
                    (*pulBinPointer)++;  ulNrExcess--;
                }
            }
            pulHisto++;
        }
    } while ((ulNrExcess) && (ulNrExcess < ulOldNrExcess));
}

void MakeHistogram(
    kz_pixel_t* pImage, 
    unsigned int uiXRes,
    unsigned int uiSizeX, 
    unsigned int uiSizeY,
    unsigned long* pulHistogram,
    unsigned int uiNrGreylevels, 
    kz_pixel_t* pLookupTable) 
{
    kz_pixel_t* pImagePointer;
    unsigned int i, j;

    for (i = 0; i < uiNrGreylevels; i++) {
        pulHistogram[i] = 0L;
    }

    for (i = 0; i < uiSizeY; i++) {
        pImagePointer = &pImage[uiSizeX];
        while (pImage < pImagePointer) {
            pulHistogram[pLookupTable[*pImage++]]++;
        }
        pImage = &pImage[uiXRes - uiSizeX];
    }
}

void MapHistogram(
    unsigned long* pulHistogram, 
    kz_pixel_t Min, 
    kz_pixel_t Max,
    unsigned int uiNrGreylevels, 
    unsigned long ulNrOfPixels)
{
    unsigned int i;  
    unsigned long ulSum = 0;
    const float fScale = ((float)(Max - Min)) / ulNrOfPixels;
    const unsigned long ulMin = (unsigned long)Min;

    for (i = 0; i < uiNrGreylevels; i++) {
        ulSum += pulHistogram[i]; 
        pulHistogram[i] = (unsigned long)(ulMin + ulSum * fScale);
        if (pulHistogram[i] > Max) {
            pulHistogram[i] = Max;
        }
    }
}

void MakeLut(
    kz_pixel_t* pLUT, 
    kz_pixel_t Min, 
    kz_pixel_t Max, 
    unsigned int uiNrBins)
{
    int i;
    const kz_pixel_t BinSize = (kz_pixel_t)(1 + (Max - Min) / uiNrBins);

    for (i = Min; i <= Max; i++) {
        pLUT[i] = (i - Min) / BinSize;
    }
}

void Interpolate(
    kz_pixel_t* pImage, 
    int uiXRes, 
    unsigned long* pulMapLU,
    unsigned long* pulMapRU, 
    unsigned long* pulMapLB, 
    unsigned long* pulMapRB,
    unsigned int uiXSize, 
    unsigned int uiYSize, 
    kz_pixel_t* pLUT)
{
    const unsigned int uiIncr = uiXRes - uiXSize;
    kz_pixel_t GreyValue; 
    unsigned int uiNum = uiXSize * uiYSize;
    unsigned int uiXCoef;
    unsigned int uiYCoef;
    unsigned int uiXInvCoef;
    unsigned int uiYInvCoef;
    unsigned int uiShift = 0;

    if (uiNum & (uiNum - 1)) {
        for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize; uiYCoef++, uiYInvCoef--, pImage += uiIncr) {
            for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize; uiXCoef++, uiXInvCoef--) {
                GreyValue = pLUT[*pImage];
                *pImage++ = (kz_pixel_t)((uiYInvCoef * (uiXInvCoef * pulMapLU[GreyValue] + uiXCoef * pulMapRU[GreyValue]) + uiYCoef * (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) / uiNum);
            }
        }
    }
    else {
        
        while (uiNum >>= 1) {
            uiShift++;
        }

        for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize; uiYCoef++, uiYInvCoef--, pImage += uiIncr) {
            for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize; uiXCoef++, uiXInvCoef--) {
                GreyValue = pLUT[*pImage];
                *pImage++ = (kz_pixel_t)((uiYInvCoef * (uiXInvCoef * pulMapLU[GreyValue] + uiXCoef * pulMapRU[GreyValue]) + uiYCoef * (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) >> uiShift);
            }
        }
    }
}

int _CLAHE(
    
    kz_pixel_t* pImage, 
    unsigned int uiXRes, 
    unsigned int uiYRes,
    kz_pixel_t Min, 
    kz_pixel_t Max, 
    unsigned int uiNrX, 
    unsigned int uiNrY,
    unsigned int uiNrBins, 
    float fCliplimit)
{
    unsigned int uiX;
    unsigned int uiY;
    unsigned int uiXSize;
    unsigned int uiYSize; 
    unsigned int uiSubX;
    unsigned int uiSubY;
    unsigned int uiXL;
    unsigned int uiXR;
    unsigned int uiYU;
    unsigned int uiYB;
    unsigned long ulClipLimit;
    unsigned long ulNrPixels;
    kz_pixel_t* pImPointer;
    kz_pixel_t aLUT[uiNR_OF_GREY];
    unsigned long* pulHist;
    unsigned long* pulMapArray;
    unsigned long* pulLU;
    unsigned long* pulLB;
    unsigned long* pulRU;
    unsigned long* pulRB;

    if (uiNrX > uiXRes) {
        return -1;
    }
    
    if (uiNrY > uiYRes) {
        return -2;
    }

    if (uiXRes % uiNrX) {
        return -3;
    }

    if (uiYRes % uiNrY) {
        return -4;
    }

    if (Max >= uiNR_OF_GREY) {
        return -5;
    }

    if (Min >= Max) {
        return -6;
    }

    if (uiNrX < 2 || uiNrY < 2) {
        return -7;
    }

    if (fCliplimit == 1.0) {
        return 0;
    }

    if (uiNrBins == 0) {
        uiNrBins = 128;
    }

    pulMapArray = (unsigned long*)malloc(sizeof(unsigned long) * uiNrX * uiNrY * uiNrBins);
    
    if (pulMapArray == 0) {
        return -8;
    }

    uiXSize = uiXRes / uiNrX; uiYSize = uiYRes / uiNrY;
    ulNrPixels = (unsigned long)uiXSize * (unsigned long)uiYSize;

    if (fCliplimit > 0.0) {
        ulClipLimit = (unsigned long)(fCliplimit * (uiXSize * uiYSize) / uiNrBins);
        ulClipLimit = (ulClipLimit < 1UL) ? 1UL : ulClipLimit;
    }
    else {
        ulClipLimit = 1UL << 14;
    }

    MakeLut(aLUT, Min, Max, uiNrBins);
    
    for (uiY = 0, pImPointer = pImage; uiY < uiNrY; uiY++) {
        for (uiX = 0; uiX < uiNrX; uiX++, pImPointer += uiXSize) {
            pulHist = &pulMapArray[uiNrBins * (uiY * uiNrX + uiX)];
            MakeHistogram(pImPointer, uiXRes, uiXSize, uiYSize, pulHist, uiNrBins, aLUT);
            ClipHistogram(pulHist, uiNrBins, ulClipLimit);
            MapHistogram(pulHist, Min, Max, uiNrBins, ulNrPixels);
        }
        pImPointer += (uiYSize - 1) * uiXRes;
    }

    for (pImPointer = pImage, uiY = 0; uiY <= uiNrY; uiY++) {
        if (uiY == 0) {
            uiSubY = uiYSize >> 1;  uiYU = 0; uiYB = 0;
        }
        else {
            if (uiY == uiNrY) {
                uiSubY = uiYSize >> 1;  uiYU = uiNrY - 1;  uiYB = uiYU;
            }
            else {
                uiSubY = uiYSize; uiYU = uiY - 1; uiYB = uiYU + 1;
            }
        }

        for (uiX = 0; uiX <= uiNrX; uiX++) {
            if (uiX == 0) {
                uiSubX = uiXSize >> 1; uiXL = 0; uiXR = 0;
            }
            else {
                if (uiX == uiNrX) {
                    uiSubX = uiXSize >> 1;  uiXL = uiNrX - 1; uiXR = uiXL;
                }
                else {
                    uiSubX = uiXSize; uiXL = uiX - 1; uiXR = uiXL + 1;
                }
            }

            pulLU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXL)];
            pulRU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXR)];
            pulLB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXL)];
            pulRB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXR)];
            Interpolate(pImPointer, uiXRes, pulLU, pulRU, pulLB, pulRB, uiSubX, uiSubY, aLUT);
            pImPointer += uiSubX; 
        }
        pImPointer += (uiSubY - 1) * uiXRes;
    }
    free(pulMapArray);      
    
    return 0;
}

int CLAHE(kz_pixel_t* input, unsigned int width, unsigned int height) 
{
    kz_pixel_t Min = 0;
    kz_pixel_t Max = uiNR_OF_GREY - 1;
    unsigned int uiNrX = 8;
    unsigned int uiNrY = 6;
    unsigned int uiNrBins = uiNR_OF_GREY;
    float fCliplimit = 4.0;

    int result = _CLAHE(input, width, height, Min, Max, uiNrX, uiNrY, uiNrBins, fCliplimit);
    if (result != 0) {
        printf("CLAHE error: %d\n", result);
        return result;
    }

    return 0;
}
