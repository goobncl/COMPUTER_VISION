#include "stdafx.h"
#include "ImageProcessor.h"


ImageProcessor::ImageProcessor(unsigned char* sharedImageArray, QObject* parent)
    : QObject(parent), targetImageArray(sharedImageArray), inputImage(Q_NULLPTR), outputImage(Q_NULLPTR), width(0), height(0)
{
    moveToThread(&workerThread);
    workerThread.start();
}

ImageProcessor::~ImageProcessor()
{
    workerThread.quit();
    workerThread.wait();
}

void ImageProcessor::setImageAndProcess(unsigned char* inputImage, int width, int height)
{
    {
        QMutexLocker locker(&mutex);
        setInputImage(inputImage, width, height);
        applyProcessing();
    }
}

void ImageProcessor::setInputImage(unsigned char* inputImage, int width, int height)
{
    this->inputImage = inputImage;
    this->width = width;
    this->height = height;
}

void ImageProcessor::applyProcessing()
{
    if (inputImage == Q_NULLPTR)
    {
        return;
    }
    gridHistogramEqualization(inputImage, width, height);
}
