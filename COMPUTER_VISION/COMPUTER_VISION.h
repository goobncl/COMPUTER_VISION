#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_COMPUTER_VISION.h"
#include <opencv2/opencv.hpp>
#include <QTimer>
#include <QLabel>


class COMPUTER_VISION : public QMainWindow
{
    Q_OBJECT

public:
    COMPUTER_VISION(QWidget *parent = Q_NULLPTR);
    ~COMPUTER_VISION();

private:
    Ui::COMPUTER_VISIONClass ui;
    QLabel* webcamLabel;
    QTimer* timer;
    cv::VideoCapture cap;
    
    void updateFrame();
};
