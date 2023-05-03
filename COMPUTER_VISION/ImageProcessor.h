#pragma once

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "img_proc.h"


class ImageProcessor : public QObject
{
    Q_OBJECT

public:
    QMutex mutex;

    explicit ImageProcessor(unsigned char* sharedImageArray, QObject* parent = Q_NULLPTR);
    ~ImageProcessor();
    
    void setImageAndProcess(unsigned char* inputImage, int width, int height);

private:
    unsigned char* targetImageArray;
    QThread workerThread;
    unsigned char* inputImage;
    unsigned char* outputImage;
    int width;
    int height;

    void setInputImage(unsigned char* inputImage, int width, int height);
    void applyProcessing();
};

#endif // IMAGEPROCESSOR_H
