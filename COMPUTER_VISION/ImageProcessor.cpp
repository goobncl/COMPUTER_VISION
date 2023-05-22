#include "stdafx.h"
#include "ImageProcessor.h"


ImgProc::ImgProc(unsigned char* sharedImageArray, QObject* parent)
    : QObject(parent), targetImageArray(sharedImageArray), inputImage(Q_NULLPTR), outputImage(Q_NULLPTR), width(0), height(0)
{
    moveToThread(&workerThread);
    workerThread.start();
}

ImgProc::~ImgProc()
{
    workerThread.quit();
    workerThread.wait();
}

void ImgProc::setImageAndProcess(unsigned char* inputImage, int width, int height, AlgType type)
{
    {
        QMutexLocker locker(&mutex);

        this->inputImage = inputImage;
        this->width = width;
        this->height = height;

        if (inputImage == Q_NULLPTR)
        {
            return;
        }

        switch (type)
        {
        case AlgType::Clahe:
            clahe(inputImage, width, height);
            break;
        case AlgType::Blur:
            gaussianBlur(inputImage, width, height);
            break;
        default:
            break;
        }
    }
}
