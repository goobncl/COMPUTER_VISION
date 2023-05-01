#pragma once

#include "stdafx.h"
#include "ui_COMPUTER_VISION.h"
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
    QLabel* webcamLabel;
    QTimer* timer;
    cv::VideoCapture cap;

    QPushButton* grayscaleBtn;
    bool isGrayscaleEnabled;

    QImage getQImageFromArray(const unsigned char* array, int width, int height);
    void updateFrame();
    void setGrayscaleEnabled(bool isEnabled);

private slots:
    void onGrayscaleBtnClicked();
};
