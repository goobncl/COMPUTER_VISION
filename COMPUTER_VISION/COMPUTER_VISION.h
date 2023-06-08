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
    unsigned char* image;
    unsigned char* rbuf;
    int* sbuf;
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
        QVector<Feature> features;
        QVector<OptFeature> optFeatures;
    };
    Data data;
    QSqlDatabase db;
    
    int sqofs;
    int nofs[4];
    Rect normrect;
    Size sz0;
    Size imgSz;
    Size minObjSz;
    Size maxObjSz;
    Size sbufSz;
    QVector<double> scales;
    QVector<ScaleData> scaleData;
    QVector<QLabel*> layerLabels;
    double varianceNormFactor;
    
    QTimer* timer;
    cv::VideoCapture cap;

    QPushButton* claheBtn;
    QPushButton* blurBtn;
    QLabel* displayLabel;
    
    bool claheEnabled;
    bool blurEnabled;
    ImgProc* imageProcessor;

    QVector<Stage> readStages(QSqlQuery& query);
    QVector<DTree> readClassifiers(QSqlQuery& query);
    QVector<DTreeNode> readNodes(QSqlQuery& query);
    QVector<float> readLeaves(QSqlQuery& query);
    QVector<Stump> readStumps(QSqlQuery& query);
    QVector<Feature> readFeatures(QSqlQuery& query);
    bool loadDataFromDB();
    void setData();

    void initImgProc();
    void initComps();
    void confCap();
    void setConn();

    void calcScales();
    bool updateScaleData();
    void computeOptFeatures();
    void computeChannels(int scaleIdx, unsigned char* img);
    double getFPS();
    void acqFrame();
    void procImg();
    void displayImg(); 
    void updateFrame();
    void verifyMatEqual(const cv::Mat& mat1, const cv::Mat& mat2, const QString& mat_name);
    void verifyIntegral(int scaleIdx);

private slots:
    void onClaheBtnClicked();
    void onBlurBtnClicked();
};
