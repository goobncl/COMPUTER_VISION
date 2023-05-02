#pragma once

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QDebug>


class ImageProcessor : public QObject
{
    Q_OBJECT

public:
    explicit ImageProcessor(unsigned char* sharedImageArray, QObject* parent = nullptr);
    ~ImageProcessor();
    void setInputImage(unsigned char* inputImage, int width, int height);
    unsigned char* getOutputImage();

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

    void exampleImageProcessingAlgorithm();
};

#endif // IMAGEPROCESSOR_H
