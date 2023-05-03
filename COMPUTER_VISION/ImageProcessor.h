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
    explicit ImageProcessor(unsigned char* sharedImageArray, QObject* parent = nullptr);
    ~ImageProcessor();
    void setInputImage(unsigned char* inputImage, int width, int height);

signals:
    void imageProcessed();

public slots:
    void processImage();

private:
    unsigned char* targetImageArray;
    QThread workerThread;
    QMutex mutex;
    unsigned char* inputImage;
    unsigned char* outputImage;
    int width;
    int height;
};

#endif // IMAGEPROCESSOR_H
