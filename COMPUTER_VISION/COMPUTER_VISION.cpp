#include "stdafx.h"
#include "COMPUTER_VISION.h"


COMPUTER_VISION::COMPUTER_VISION(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    initImgProc();

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    displayLabel = findChild<QLabel*>("videoLabel");
    claheBtn = findChild<QPushButton*>("claheBtn");
    blurBtn = findChild<QPushButton*>("blurBtn");
    
    claheEnabled = false;
    blurEnabled = false;

    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &COMPUTER_VISION::updateFrame);
    connect(claheBtn, &QPushButton::clicked, this, &COMPUTER_VISION::onClaheBtnClicked);
    connect(blurBtn, &QPushButton::clicked, this, &COMPUTER_VISION::onBlurBtnClicked);

    cap.open(0);
    timer->start(0);
}

COMPUTER_VISION::~COMPUTER_VISION()
{
    delete[] imageArray;
}

void COMPUTER_VISION::calcScales()
{
    QVector<double> allScales;

    for (double factor = 1; ; factor *= 1.1f) {
        Size winSz = Size(
            doubleRound(origWinSz.width * factor),
            doubleRound(origWinSz.height * factor));
        if (winSz.width > imgSz.width || winSz.height > imgSz.height) {
            break;
        }
        allScales.append(factor);
    }

    for (size_t index = 0; index < allScales.size(); index++) {
        Size winSz = Size(
            doubleRound(origWinSz.width * allScales[index]),
            doubleRound(origWinSz.height * allScales[index]));
        if (winSz.width > maxObjSz.width || winSz.height > maxObjSz.height) {
            break;
        }
        if (winSz.width < minObjSz.width || winSz.height < minObjSz.height) {
            continue;
        }
        scales.append(allScales[index]);
    }
}

bool COMPUTER_VISION::updateScaleData()
{
    if (scaleData.isEmpty()) {
        scaleData = QVector<ScaleData>();
    }

    size_t nscales = scales.size();
    bool recalcOptFeatures = (nscales != scaleData.size());
    scaleData.resize(nscales);

    int layerDeltaY = 0;
    Point layerOffset(0, 0);
    Size prevBuffSz = sbufSz;

    int alignedWidth = (int)alignSize(doubleRound(imgSz.width / scales[0]) + 31, 32);
    sbufSz.width = (sbufSz.width > alignedWidth) ? sbufSz.width : alignedWidth;
    recalcOptFeatures = recalcOptFeatures || (sbufSz.width != prevBuffSz.width);

    for (size_t i = 0; i < nscales; i++) {

        ScaleData& s = scaleData[i];

        if (!recalcOptFeatures && doubleAbs(s.scale - scales[i]) > (FLT_EPSILON * 100 * scales[i])) {
            recalcOptFeatures = true;
        }

        double sc = scales[i];
        Size sz;
        sz.width = doubleRound(imgSz.width / sc);
        sz.height = doubleRound(imgSz.height / sc);
        s.ystep = (sc >= 2) ? (1) : (2);
        s.scale = sc;
        s.szi = Size(sz.width + 1, sz.height + 1);

        if (i == 0) {
            layerDeltaY = s.szi.height;
        }

        if (layerOffset.x + s.szi.width > sbufSz.width) {
            layerOffset = Point(0, layerOffset.y + layerDeltaY);
            layerDeltaY = s.szi.height;
        }

        s.layer_offset = layerOffset.y * sbufSz.width + layerOffset.x;
        layerOffset.x += s.szi.width;
    }

    layerOffset.y += layerDeltaY;
    sbufSz.height = (sbufSz.height > layerOffset.y) ? (sbufSz.height) : (layerOffset.y);
    recalcOptFeatures = recalcOptFeatures || (sbufSz.height != prevBuffSz.height);

    return recalcOptFeatures;
}

void COMPUTER_VISION::initImgProc()
{
    // TODO: Haar Feature metadata
    imgSz = Size(640, 480);
    origWinSz = Size(24, 24);
    minObjSz = Size(30, 30);
    maxObjSz = imgSz;
    sbufSz = Size(0, 0);

    calcScales();
    bool recalcOptFeatures = updateScaleData();

    imageArray = new unsigned char[imgSz.width * imgSz.height];
    imageProcessor = new ImgProc(imageArray, this);
}


double COMPUTER_VISION::getFPS() {
    static double fps = 0.0;
    static std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
    static int frames = 0;
    frames++;
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = now - lastTime;
    if (duration.count() >= 1.0) {
        fps = frames / duration.count();
        frames = 0;
        lastTime = now;
    }
    return fps;
}

void COMPUTER_VISION::updateFrame()
{
    cv::Mat frame;
    cap >> frame;

    if (frame.empty())
    {
        timer->stop();
        return;
    }

    // Image Acquisition
    cv::Mat grayFrame;
    cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

    {
        QMutexLocker locker(&imageProcessor->mutex);
        memcpy(imageArray, grayFrame.data, 640 * 480);
    }

    // Image processing
    if (claheEnabled) 
    {
        imageProcessor->setImageAndProcess(imageArray, 640, 480, ImgProc::AlgType::Clahe);
    }

    if (blurEnabled) {
        imageProcessor->setImageAndProcess(imageArray, 640, 480, ImgProc::AlgType::Blur);
    }

    // Image Display
    QImage qFrame = QImage(imageArray, 640, 480, QImage::Format_Grayscale8).copy();
    displayLabel->setPixmap(QPixmap::fromImage(qFrame));

    QString fpsString = QString::number(getFPS(), 'f', 8);
    statusBar()->showMessage("FPS: " + fpsString);
}

void COMPUTER_VISION::onClaheBtnClicked()
{
    claheEnabled = !claheEnabled;

    QFont font = claheBtn->font();
    if (claheEnabled)
    {
        font.setWeight(QFont::Bold);
    }
    else
    {
        font.setWeight(QFont::Normal);
    }
    claheBtn->setFont(font);
}

void COMPUTER_VISION::onBlurBtnClicked()
{
    blurEnabled = !blurEnabled;
	QFont font = blurBtn->font();
    if (blurEnabled)
    {
		font.setWeight(QFont::Bold);
	}
    else
    {
		font.setWeight(QFont::Normal);
	}
	blurBtn->setFont(font);
}

size_t COMPUTER_VISION::alignSize(size_t sz, int n)
{
    return (sz + n - 1) & -n;
}

double COMPUTER_VISION::doubleAbs(double n)
{
    return (n < 0) ? (-n) : (n);
}
