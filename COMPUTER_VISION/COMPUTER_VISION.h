#pragma once

#include "stdafx.h"
#include "ui_COMPUTER_VISION.h"
#include "ImageProcessor.h"
#include "haarFeature.h"
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
    
    // TODO: Haar Feature metadata
    Size imgSz;
    Size origWinSz;
    Size minObjSz;
    Size maxObjSz;

    Ui::COMPUTER_VISIONClass ui;
    
    QTimer* timer;
    cv::VideoCapture cap;

    QPushButton* claheBtn;
    QPushButton* blurBtn;
    QLabel* displayLabel;
    
    bool claheEnabled;
    bool blurEnabled;
    ImgProc* imageProcessor;

    void initImgProc();
    double getFPS();
    void updateFrame();    

private slots:
    void onClaheBtnClicked();
    void onBlurBtnClicked();
};
