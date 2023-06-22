#pragma once

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include "clahe.h"
#include "gaussianBlur.h"
#include "haarFeature.h"
#include "CascadeClassifier.h"


class ImgProc : public QObject
{
    Q_OBJECT

public:
    QMutex mutex;

    enum class AlgType
    {
        Clahe,
        Blur,
        Face
    };

    explicit ImgProc(QObject* parent = Q_NULLPTR);
    ~ImgProc();
    
    void setImageAndProcess(unsigned char* inputImage, int width, int height, AlgType type);

private:
    QThread workerThread;
    CascadeClassifier faceDetector;
};

#endif // IMAGEPROCESSOR_H
