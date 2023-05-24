#pragma once

#include "stdafx.h"
#include "ui_COMPUTER_VISION.h"
#include "ImageProcessor.h"


class COMPUTER_VISION : public QMainWindow
{
    Q_OBJECT

public:
    COMPUTER_VISION(QWidget* parent = Q_NULLPTR);
    ~COMPUTER_VISION();

private:
    unsigned char* imageArray;
    
    // TODO: Haar Feature metadata
    Ui::COMPUTER_VISIONClass ui;
    
    Size imgSz;
    Size origWinSz;
    Size minObjSz;
    Size maxObjSz;
    Size sbufSz;
    QVector<double> scales;
    QVector<ScaleData> scaleData;
    cv::Mat rbuf, sbuf;
    
    QTimer* timer;
    cv::VideoCapture cap;

    QPushButton* claheBtn;
    QPushButton* blurBtn;
    QLabel* displayLabel;
    
    bool claheEnabled;
    bool blurEnabled;
    ImgProc* imageProcessor;

    void calcScales();
    bool updateScaleData();
    void initImgProc();
    double getFPS();
    void updateFrame();
    size_t alignSize(size_t sz, int n);
    double doubleAbs(double n);


private slots:
    void onClaheBtnClicked();
    void onBlurBtnClicked();
};
