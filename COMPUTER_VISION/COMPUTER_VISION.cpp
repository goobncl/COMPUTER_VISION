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

    {
        for (int i = 0; i < 30; ++i) {
            QString objectName = "layerLabel_" + QString::number(i).rightJustified(2, '0');
            QLabel* label = findChild<QLabel*>(objectName);
            if (label) {
                layerLabels.push_back(label);
            }
        }
    }
    
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
    free(imageArray);    
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

Size COMPUTER_VISION::clacSz0(Size oriSz, ImgLayer& rbuf)
{
    int alignedSizeWidth = alignSize(oriSz.width, 16);
    alignedSizeWidth = rbuf.sz.width > alignedSizeWidth ? rbuf.sz.width : alignedSizeWidth;
    int maxHeight = rbuf.sz.height > oriSz.height ? rbuf.sz.height : oriSz.height;
    return Size(alignedSizeWidth, maxHeight);
}

void COMPUTER_VISION::buildImgPyramid()
{
    Size sz0 = clacSz0(scaleData.at(0).szi, resizedBuf);
    int nscales = scaleData.size();
    imgPyramid.resize(nscales);

    QFutureWatcher<void> watcher;
    QList<QFuture<void>> futures;

    for (int i = 0; i < nscales; i++) {
        futures.append(QtConcurrent::run([this, i] {
            const ScaleData& s = scaleData.at(i);
            int new_w = s.szi.width - 1;
            int new_h = s.szi.height - 1;

            unsigned char* output = downSampling(imageArray, new_w, new_h);

            imgPyramid[i].sz.width = new_w;
            imgPyramid[i].sz.height = new_h;
            imgPyramid[i].data = output;
         }));
    }

    for (auto& future : futures) {
        watcher.setFuture(future);
        watcher.waitForFinished();
    }
}

void COMPUTER_VISION::clearImgPyramid()
{
    for (int i = 0; i < imgPyramid.size(); i++) {
		free(imgPyramid[i].data);
	}
    imgPyramid.clear();
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

    if (recalcOptFeatures) {
        //computeOptFeatures();
    }

    imageArray = (unsigned char*)malloc(sizeof(unsigned char) * imgSz.width * imgSz.height);
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

void COMPUTER_VISION::acqFrame()
{
    cv::Mat frame;
    cv::Mat grayFrame;
    cap >> frame;
    if (frame.empty()) timer->stop();
    cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

    {
        QMutexLocker locker(&imageProcessor->mutex);
        memcpy(imageArray, grayFrame.data, FRAME_W * FRAME_H);
    }
}

void COMPUTER_VISION::procImg()
{
    if (claheEnabled) {
        imageProcessor->setImageAndProcess(
            imageArray, FRAME_W, FRAME_H, 
            ImgProc::AlgType::Clahe
        );
    }   

    if (blurEnabled) {
        imageProcessor->setImageAndProcess(
            imageArray, FRAME_W, FRAME_H, 
            ImgProc::AlgType::Blur
        );
    }   
}

void COMPUTER_VISION::displayImg()
{
    cv::Mat cvFrame;
    {
        QMutexLocker locker(&imageProcessor->mutex);
        cvFrame = cv::Mat(FRAME_H, FRAME_W, CV_8UC1, imageArray).clone();
    }
    QImage qFrame = QImage(cvFrame.data, cvFrame.cols, cvFrame.rows, cvFrame.step, QImage::Format_Grayscale8);
    displayLabel->setPixmap(QPixmap::fromImage(qFrame));

    QString fpsString = QString::number(getFPS(), 'f', 8);
    statusBar()->showMessage("FPS: " + fpsString);
}

void COMPUTER_VISION::displayPyramid() 
{
    int nLayers = imgPyramid.size();
    for (size_t i = 0; i < nLayers; ++i) {
        cv::Mat cvImage(imgPyramid[i].sz.height, imgPyramid[i].sz.width, CV_8U, imgPyramid[i].data);
        QImage image = QImage(cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_Grayscale8);
        QPixmap pixmap = QPixmap::fromImage(image);
        QLabel* label = layerLabels[i];
        pixmap = pixmap.scaled(label->width(), label->height(), Qt::KeepAspectRatioByExpanding);
        label->setPixmap(pixmap);
    }
}

void COMPUTER_VISION::updateFrame()
{
    acqFrame();
    procImg();
    displayImg();
    buildImgPyramid();
    displayPyramid();
    clearImgPyramid();    
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
