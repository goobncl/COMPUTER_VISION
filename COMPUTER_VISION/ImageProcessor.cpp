#include "stdafx.h"
#include "ImageProcessor.h"


ImageProcessor::ImageProcessor(unsigned char* sharedImageArray, QObject* parent)
    : QObject(parent), sharedImageArray(sharedImageArray), inputImage(nullptr), outputImage(nullptr), width(0), height(0)
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
    qDebug() << "[1] -- Start on thread ID:" << QThread::currentThreadId();

    // Your image processing algorithm goes here.
    // For example:
    for (int i = 0; i < width * height; ++i)
    {
        sharedImageArray[i] = 255 - sharedImageArray[i];
    }

    qDebug() << "[2] @@ Finish on thread ID:" << QThread::currentThreadId();
}
