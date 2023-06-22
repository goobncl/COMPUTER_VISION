#include "stdafx.h"
#include "ImageProcessor.h"


ImgProc::ImgProc(QObject* parent)
    : QObject(parent)
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
        case AlgType::Face:

            break;
        default:
            break;
        }
    }
}
