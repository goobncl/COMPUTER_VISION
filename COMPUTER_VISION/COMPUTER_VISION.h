#pragma once

#include "stdafx.h"
#include "ui_COMPUTER_VISION.h"
#include "util.h"
#include "ImageProcessor.h"

constexpr int FRAME_W = 640;
constexpr int FRAME_H = 480;


enum class LayerState {
    DATA, SUM, SQSUM
};

struct ImgLayer {
    Size sz;
    unsigned char* data;
    int* sum;
    int* sqsum;
    LayerState state;
    double varNFact;
    ImgLayer() : sz(Size(0, 0)), data(NULL), sum(NULL), sqsum(NULL), state(LayerState::DATA), varNFact(0.f) {}
};

class COMPUTER_VISION : public QMainWindow
{
    Q_OBJECT

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

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
    QVector<ImgLayer> imgPyramid;
    
    QVector<Rect> faces;
    QMutex facesMutex;
    
    QTimer* timer;
    cv::VideoCapture cap;

    QLineSeries* series;
    QChartView* fpsTimeSeries;
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
    void initComponents();
    QLabel* createNumLabel(QLabel* label, int i);
    QGraphicsDropShadowEffect* createDropShadowEffect();
    void initLayerLabels();
    void customizeAxis(QChart* chart);
    void setAxisStyle(QValueAxis* axis);
    void initFpsTimeSeries();
    void initStatusBar();

    void initComps();
    void setCam();
    void setConn();

    Size clacSz0(Size oriSz);
    void calcImgPyramid();
    QImage normMat(cv::Mat& cvImage);
    void displayLayer(ImgLayer& layer, int layerIndex);    
    double calcNormFactor(int* pSum, int* pSqsum, int x, int y, int width);
    void calcHaarFeature();
    bool compRect(const Rect& r1, const Rect& r2);
    void clearImgPyramid();
    void displayPyramid();

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
    void saveMatToCsv(const cv::Mat& mat, const QString& filename);
    int predictOrderedStump(int* ptr, int width, int height, double varNFact);
    bool setWindow(int* ptr, int scaleIdx);

private slots:
    void onClaheBtnClicked();
    void onBlurBtnClicked();
    void onLayerClicked(int layerIndex);
};
