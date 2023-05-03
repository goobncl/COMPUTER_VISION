#include "stdafx.h"
#include "ImageProcessor.h"


ImageProcessor::ImageProcessor(unsigned char* sharedImageArray, QObject* parent)
    : QObject(parent), targetImageArray(sharedImageArray), inputImage(nullptr), outputImage(nullptr), width(0), height(0)
{
    moveToThread(&workerThread);
    workerThread.start();
}

ImageProcessor::~ImageProcessor()
{
    workerThread.quit();
    workerThread.wait();
}

void ImageProcessor::setInputImage(unsigned char* inputImage, int width, int height)
{
    QMutexLocker locker(&mutex);
    this->inputImage = inputImage;
    this->width = width;
    this->height = height;
}

void ImageProcessor::processImage()
{
    {
        QMutexLocker locker(&mutex);
        if (inputImage == nullptr)
        {
            return;
        }
        invertColors(inputImage, width, height);
    }
}
