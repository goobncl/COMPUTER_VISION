#pragma once

#include "stdafx.h"
#include "ui_COMPUTER_VISION.h"
#include "util.h"
#include "ImageProcessor.h"


class COMPUTER_VISION : public QMainWindow
{
    Q_OBJECT

public:
    COMPUTER_VISION(QWidget* parent = Q_NULLPTR);
    ~COMPUTER_VISION();

private:
    unsigned char* imageArray;
    Ui::COMPUTER_VISIONClass ui;
    
    // TODO: Haar Feature metadata
    Size imgSz;
    Size origWinSz;
    Size minObjSz;
    Size maxObjSz;
    Size sbufSz;
    ImgLayer resizedBuf;
    QVector<ImgLayer> imgPyramid;
    QVector<double> scales;
    QVector<ScaleData> scaleData;
    
    
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
    Size clacSz0(Size oriSz, ImgLayer& resizedBuf);
    void initImgProc();
    double getFPS();
    void updateFrame();


private slots:
    void onClaheBtnClicked();
    void onBlurBtnClicked();
};
