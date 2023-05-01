#pragma once

#include "stdafx.h"
#include "ui_COMPUTER_VISION.h"
#include "ImageProcessor.h"
#include <QTimer>
#include <QLabel>


class COMPUTER_VISION : public QMainWindow
{
    Q_OBJECT

public:
    COMPUTER_VISION(QWidget* parent = Q_NULLPTR);
    ~COMPUTER_VISION();

private:
    unsigned char* imageArray;

    Ui::COMPUTER_VISIONClass ui;
    QLabel* displayLabel;
    QTimer* timer;
    cv::VideoCapture cap;

    QPushButton* claheBtn;
    bool isGrayscaleEnabled;
    
    ImageProcessor* imageProcessor;

    double getFPS();
    void updateFrame();
    void setGrayscaleEnabled(bool isEnabled);

private slots:
    void onClaheBtnClicked();
    void onImageProcessed();
};
