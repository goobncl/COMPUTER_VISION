#include "stdafx.h"
#include "ImageProcessor.h"


ImageProcessor::ImageProcessor(QObject* parent)
    : QObject(parent), inputImage(nullptr), outputImage(nullptr), width(0), height(0)
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

unsigned char* ImageProcessor::getOutputImage()
{
    QMutexLocker locker(&mutex);
    return outputImage;
}

void ImageProcessor::processImage()
{
    {
        QMutexLocker locker(&mutex);
        if (inputImage == nullptr)
        {
            return;
        }
        exampleImageProcessingAlgorithm();
    }
    emit imageProcessed();
}

void ImageProcessor::exampleImageProcessingAlgorithm()
{
    // Your image processing algorithm goes here.
    // For example:
    outputImage = new unsigned char[width * height];
    for (int i = 0; i < width * height; i++)
    {
        outputImage[i] = static_cast<unsigned char>(inputImage[i] / 2);
    }
}
