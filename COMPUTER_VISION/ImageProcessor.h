#pragma once

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QThread>
#include <QMutex>

class ImageProcessor : public QObject
{
    Q_OBJECT

public:
    explicit ImageProcessor(QObject* parent = nullptr);
    ~ImageProcessor();
    void setInputImage(unsigned char* inputImage, int width, int height);
    unsigned char* getOutputImage();

signals:
    void imageProcessed();

public slots:
    void processImage();

private:
    QThread workerThread;
    QMutex mutex;
    unsigned char* inputImage;
    unsigned char* outputImage;
    int width;
    int height;

    void exampleImageProcessingAlgorithm();
};

#endif // IMAGEPROCESSOR_H
