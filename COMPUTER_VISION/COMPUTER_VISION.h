#pragma once

#include "stdafx.h"
#include "ui_COMPUTER_VISION.h"
#include "util.h"
#include "ImageProcessor.h"

constexpr int FRAME_W = 640;
constexpr int FRAME_H = 480;


class COMPUTER_VISION : public QMainWindow
{
    Q_OBJECT

public:
    COMPUTER_VISION(QWidget* parent = Q_NULLPTR);
    ~COMPUTER_VISION();

private:
    unsigned char* imageArray;
    Ui::COMPUTER_VISIONClass ui;
        
    struct Data {

        int minNodesPerTree;
        int maxNodesPerTree;
        Size origWinSz;
        QVector<Stage> stages;
        QVector<DTree> classifiers;
        QVector<DTreeNode> nodes;
        QVector<float> leaves;
        QVector<Stump> stumps;
    };
    Data data;
    QSqlDatabase db;
    
    Size imgSz;
    Size origWinSz;
    Size minObjSz;
    Size maxObjSz;
    Size sbufSz;
    QVector<double> scales;
    QVector<ScaleData> scaleData;
    QVector<QLabel*> layerLabels;
    ImgLayer resizedBuf;
    QVector<ImgLayer> imgPyramid;
    
    QTimer* timer;
    cv::VideoCapture cap;

    QPushButton* claheBtn;
    QPushButton* blurBtn;
    QLabel* displayLabel;
    
    bool claheEnabled;
    bool blurEnabled;
    ImgProc* imageProcessor;

    void initImgProc();
    void initComps();
    void confCap();
    void setConn();

    QVector<Stage> readStages(QSqlQuery& query);
    QVector<DTree> readClassifiers(QSqlQuery& query);
    QVector<DTreeNode> readNodes(QSqlQuery& query);
    QVector<float> readLeaves(QSqlQuery& query);
    QVector<Stump> readStumps(QSqlQuery& query);
    bool loadDataFromDB();
    void setData();

    void calcScales();
    bool updateScaleData();
    Size clacSz0(Size oriSz, ImgLayer& resizedBuf);
    double getFPS();
    void acqFrame();
    void procImg();
    void displayImg();  
    void buildImgPyramid();
    void displayPyramid();
    void clearImgPyramid();
    void updateFrame();


private slots:
    void onClaheBtnClicked();
    void onBlurBtnClicked();
};
