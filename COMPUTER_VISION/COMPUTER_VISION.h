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

public:
    COMPUTER_VISION(QWidget* parent = Q_NULLPTR);
    ~COMPUTER_VISION();

private:
    Ui::COMPUTER_VISIONClass ui;    
    unsigned char* image;
    Size imgSz;        
    ImgProc* imageProcessor;
    const std::vector<Rect>* faces;
    QTimer* timer;
    cv::VideoCapture cap;
    QLineSeries* series;
    QChartView* fpsTimeSeries;
    QPushButton* claheBtn;
    QPushButton* blurBtn;
    QPushButton* faceBtn;
    QLabel* displayLabel;        
    bool claheEnabled;
    bool blurEnabled;
    bool faceEnabled;
    
    void initImgProc();
    void initComponents();
    void customizeAxis(QChart* chart);
    void setAxisStyle(QValueAxis* axis);
    void initStatusBar();
    void initFpsTimeSeries();
    void initComps();
    void setCam();
    void setConn();        
    void drawFaces();     
    double getFPS();
    void acqFrame();
    void procImg();
    void displayImg(); 
    void updateFrame();
    

private slots:
    void onClaheBtnClicked();
    void onBlurBtnClicked();
    void onFaceBtnClicked();    

#if 0:
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
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
    unsigned char* rbuf;
    int* sbuf;
    int sqofs;
    int nofs[4];
    Rect normrect;
    Size sz0;
    Size sbufSz;
    QVector<double> scales;
    QVector<ScaleData> scaleData;
    QVector<QLabel*> layerLabels;
    QVector<ImgLayer> imgPyramid;

    void setData();
    void initLayerLabels();
    QLabel* createNumLabel(QLabel* label, int i);
    QGraphicsDropShadowEffect* createDropShadowEffect();
    Size clacSz0(Size oriSz);
    void computeChannels(int scaleIdx, unsigned char* img);
    void computeOptFeatures();
    bool setWindow(int* ptr, int scaleIdx);
    void verifyIntegral(int scaleIdx);
    void displayLayer(ImgLayer& layer, int layerIndex);
    void displayPyramid();

private slots:
    void onLayerClicked(int layerIndex);

#endif
};
