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
    

    
    Size clacSz0(Size oriSz);
    QImage normMat(cv::Mat& cvImage);
    void displayLayer(ImgLayer& layer, int layerIndex);    
    void displayPyramid();
    void drawFaces();
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
    bool setWindow(int* ptr, int scaleIdx);

#if 1:
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
#endif

private slots:
    void onClaheBtnClicked();
    void onBlurBtnClicked();
    void onFaceBtnClicked();
    void onLayerClicked(int layerIndex);
};
