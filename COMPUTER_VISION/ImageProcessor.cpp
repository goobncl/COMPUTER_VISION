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
        case AlgType::Face:
            clahe(inputImage, width, height);
            faceDetector.calcImgPyramid(inputImage);
            faceDetector.calcHaarFeature();
            faceDetector.groupRectangles(faceDetector.candidates, 10, 0.2);
            break;
        case AlgType::Blur:
            gaussianBlur(inputImage, width, height);
            break;
        default:
            break;
        }
    }
}
