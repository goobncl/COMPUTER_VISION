#pragma once

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "CLAHE.h"
#include "GAUSSIAN_BLUR.h"


class ImgProc : public QObject
{
    Q_OBJECT

public:
    QMutex mutex;

    enum class AlgType
    {
        Clahe,
        Blur
    };

    explicit ImgProc(unsigned char* sharedImageArray, QObject* parent = Q_NULLPTR);
    ~ImgProc();
    
    void setImageAndProcess(unsigned char* inputImage, int width, int height, AlgType type);

private:
    unsigned char* targetImageArray;
    QThread workerThread;
    unsigned char* inputImage;
    unsigned char* outputImage;
    int width;
    int height;
};

#endif // IMAGEPROCESSOR_H
