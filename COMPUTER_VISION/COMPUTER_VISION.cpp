#include "stdafx.h"
#include "COMPUTER_VISION.h"


COMPUTER_VISION::COMPUTER_VISION(QWidget* parent)
    : QMainWindow(parent),
    claheEnabled(false),
    blurEnabled(false),
    faceEnabled(false),
    faces(NULL)
{
    ui.setupUi(this);

    initImgProc();
    initComps();
    setCam();
    setConn();
}

COMPUTER_VISION::~COMPUTER_VISION()
{
    cap.release();
    delete timer;
    delete imageProcessor;
    free(image);
}

void COMPUTER_VISION::initComponents()
{
    displayLabel = findChild<QLabel*>("videoLabel");
    claheBtn = findChild<QPushButton*>("claheBtn");
    blurBtn = findChild<QPushButton*>("blurBtn");
    faceBtn = findChild<QPushButton*>("faceBtn");
}

void COMPUTER_VISION::initFpsTimeSeries()
{
    fpsTimeSeries = findChild<QChartView*>("fpsTimeSerise");
    QChart* chart = new QChart();
    series = new QLineSeries();

    QPen pen(Qt::red);
    pen.setWidth(2);
    series->setPen(pen);

    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setPlotArea(QRect(30, 20, 590, 150));
    chart->setBackgroundBrush(Qt::black);
    fpsTimeSeries->setBackgroundBrush(Qt::black);
    customizeAxis(chart);
    chart->legend()->setVisible(false);
    fpsTimeSeries->setChart(chart);
}

void COMPUTER_VISION::setAxisStyle(QValueAxis* axis)
{
    QPen gridPen(Qt::white);
    gridPen.setStyle(Qt::DotLine);
    axis->setGridLinePen(gridPen);

    QPen axisPen(Qt::white);
    axis->setLinePen(axisPen);
    axis->setLabelsColor(Qt::white);
    axis->setGridLineColor(Qt::white);
}

void COMPUTER_VISION::customizeAxis(QChart* chart)
{
    QValueAxis* axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).at(0));
    QValueAxis* axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).at(0));
    axisY->setLabelsAngle(-90);
    axisY->setRange(0, 40.0);
    axisX->setTickCount(8);
    axisX->setLabelsVisible(false);
    setAxisStyle(axisX);
    setAxisStyle(axisY);
}

void COMPUTER_VISION::initStatusBar()
{
    this->statusBar()->setStyleSheet("QStatusBar{border-top: 1px solid #747574;}");
    this->statusBar()->setSizeGripEnabled(false);
}

void COMPUTER_VISION::initComps()
{
    initComponents();
    initFpsTimeSeries();
    initStatusBar();
}

void COMPUTER_VISION::setCam()
{
    cap.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_W);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_H);
    cap.open(0);
}

void COMPUTER_VISION::setConn()
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &COMPUTER_VISION::updateFrame);
    connect(claheBtn, &QPushButton::clicked, this, &COMPUTER_VISION::onClaheBtnClicked);
    connect(blurBtn, &QPushButton::clicked, this, &COMPUTER_VISION::onBlurBtnClicked);
    connect(faceBtn, &QPushButton::clicked, this, &COMPUTER_VISION::onFaceBtnClicked);

    timer->start(0);
}

void COMPUTER_VISION::initImgProc()
{
    imgSz = Size(FRAME_W, FRAME_H);

    image = (unsigned char*)malloc(sizeof(unsigned char) * imgSz.width * imgSz.height);
    imageProcessor = new ImgProc(this);
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
        memcpy(image, grayFrame.data, FRAME_W * FRAME_H);
    }
}

void COMPUTER_VISION::procImg()
{
    if (claheEnabled) {
        imageProcessor->setImageAndProcess(
            image, 
            FRAME_W, 
            FRAME_H, 
            ImgProc::AlgType::Clahe
        );
    }   

    if (blurEnabled) {
        imageProcessor->setImageAndProcess(
            image, 
            FRAME_W, 
            FRAME_H, 
            ImgProc::AlgType::Blur
        );
    }

    if (faceEnabled) {
        std::any result = imageProcessor->setImageAndProcess(
            image,
            FRAME_W,
            FRAME_H,
            ImgProc::AlgType::Face
        );

        if (result.has_value()) {
            faces = std::any_cast<const std::vector<Rect>*>(result);
        }
    }
}

void COMPUTER_VISION::displayImg() {
    cv::Mat cvFrame;
    {
        QMutexLocker locker(&imageProcessor->mutex);
        cvFrame = cv::Mat(FRAME_H, FRAME_W, CV_8UC1, image);
    }
    QImage qFrame(cvFrame.data, cvFrame.cols, cvFrame.rows, cvFrame.step, QImage::Format_Grayscale8);
    displayLabel->setPixmap(QPixmap::fromImage(std::move(qFrame)));

    {
        double fps = getFPS();
        QTime now = QTime::currentTime();
        int timeNow = now.msecsSinceStartOfDay();
        series->append(timeNow, fps);

        while (series->count() > 0 && timeNow - series->at(0).x() > 2000) {
            series->remove(0);
        }

        QValueAxis* axisX = qobject_cast<QValueAxis*>(fpsTimeSeries->chart()->axisX());
        axisX->setRange(timeNow - 1000, timeNow);

        statusBar()->showMessage(QString("FPS: %1").arg(fps, 0, 'f', 8));
    }
}

void COMPUTER_VISION::updateFrame()
{
    acqFrame();
    procImg();
    displayImg(); 

    if (faceEnabled) {
        drawFaces();
    }
}

void COMPUTER_VISION::drawFaces()
{
    if ((!faces || faces->empty())) {
        return;
    }

    QPixmap pixmap = displayLabel->pixmap();
    QPainter painter(&pixmap);

    QPen pen;
    pen.setWidth(5);
    pen.setColor(Qt::green);
    painter.setPen(pen);

    for (const Rect& face : *faces) {
        painter.drawRect(face.x, face.y, face.width, face.height);
    }

    displayLabel->setPixmap(pixmap);
}

void COMPUTER_VISION::onClaheBtnClicked()
{
    claheEnabled = !claheEnabled;

    QFont font = claheBtn->font();
    
    if (claheEnabled) {
        font.setWeight(QFont::Bold);
    }
    else {
        font.setWeight(QFont::Normal);
    }
    
    claheBtn->setFont(font);
}

void COMPUTER_VISION::onBlurBtnClicked()
{
    blurEnabled = !blurEnabled;
	QFont font = blurBtn->font();
    
    if (blurEnabled) {
		font.setWeight(QFont::Bold);
	}
    else {
		font.setWeight(QFont::Normal);
	}
	
    blurBtn->setFont(font);
}

void COMPUTER_VISION::onFaceBtnClicked()
{
    faceEnabled = !faceEnabled;
    QFont font = faceBtn->font();

    if (faceEnabled) {
        font.setWeight(QFont::Bold);
    }
    else {
        font.setWeight(QFont::Normal);
    }

    faceBtn->setFont(font);
}

#if 0:

bool COMPUTER_VISION::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        for (size_t i = 0; i < layerLabels.size(); ++i) {
            if (obj == layerLabels[i]) {
                onLayerClicked(i);
                return true;
            }
        }
    }

    return QObject::eventFilter(obj, event);
}

void COMPUTER_VISION::setData()
{
    data.minNodesPerTree = 1;
    data.maxNodesPerTree = 1;
    data.origWinSz = Size(24, 24);
    normrect = Rect(1, 1, data.origWinSz.width - 2, data.origWinSz.height - 2);
}

void COMPUTER_VISION::initLayerLabels()
{
    int nLayers = scales.size();
    for (size_t i = 0; i < nLayers; ++i) {
        QString objectName = "layerLabel_" + QString::number(i).rightJustified(2, '0');
        QLabel* label = findChild<QLabel*>(objectName);

        if (label) {
            layerLabels.push_back(label);
            label->installEventFilter(this);
            QLabel* numLabel = createNumLabel(label, i);
            QGraphicsDropShadowEffect* effect = createDropShadowEffect();
            numLabel->setGraphicsEffect(effect);
        }
    }
}

QLabel* COMPUTER_VISION::createNumLabel(QLabel* label, int i)
{
    QLabel* numLabel = new QLabel(label);
    QString labelText = "[" + QString::number(i + 1) + "] ";
    labelText += QString::number(scaleData[i].szi.width);
    labelText += " x ";
    labelText += QString::number(scaleData[i].szi.height);
    numLabel->setText(labelText);

    numLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    numLabel->setContentsMargins(5, 5, 5, 5);
    numLabel->setStyleSheet("QLabel {"
        " color : yellow;"
        " border: none;"
        " font-weight: bold;"
        " font-size: 10px;"
        "}");

    return numLabel;
}

QGraphicsDropShadowEffect* COMPUTER_VISION::createDropShadowEffect()
{
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(2);
    effect->setXOffset(1);
    effect->setYOffset(1);
    effect->setColor(QColor("black"));

    return effect;
}

Size COMPUTER_VISION::clacSz0(Size oriSz)
{
    int alignedSizeWidth = alignSize(oriSz.width, 16);
    return Size(alignedSizeWidth, oriSz.height);
}

void COMPUTER_VISION::computeChannels(int scaleIdx, unsigned char* img)
{
    const ScaleData& s = scaleData.at(scaleIdx);
    sqofs = sbufSz.area();
    integral(img, sbuf, s.szi.width, s.szi.height, s.layer_offset);
    integralSquare(img, sbuf, s.szi.width, s.szi.height, (s.layer_offset + sqofs));
}

void COMPUTER_VISION::computeOptFeatures()
{
    int sstep = sbufSz.width;
    CV_SUM_OFS(nofs[0], nofs[1], nofs[2], nofs[3], 0, normrect, sstep);
    size_t nfeatures = data.features.size();
    QVector<Feature>& ff = data.features;
    data.optFeatures.resize(nfeatures);
    OptFeature* optfeaturesPtr = &(data.optFeatures)[0];
    for (size_t fi = 0; fi < nfeatures; fi++) {
        optfeaturesPtr[fi].setOffsets(ff[fi], sstep);
    }
}

bool COMPUTER_VISION::setWindow(int* ptr, int scaleIdx)
{
    const ScaleData& s = scaleData.at(scaleIdx);

    const int* pwin = ptr + s.layer_offset;
    const int* pq = (const int*)(pwin + sqofs);
    int valsum = CALC_SUM_OFS(nofs, pwin);
    unsigned valsqsum = (unsigned)(CALC_SUM_OFS(nofs, pq));

    double area = normrect.area();
    double nf = area * valsqsum - (double)valsum * valsum;
    if (nf > 0.) {
        nf = std::sqrt(nf);
        int varNormFact = (float)(1. / nf);
        return area * varNormFact < 1e-1;
    }
    else {
        int varNormFact = 1.f;
        return false;
    }
}

void COMPUTER_VISION::onLayerClicked(int layerIndex) {
    ImgLayer& layer = imgPyramid[layerIndex];
    switch (layer.state) {
    case LayerState::DATA:
        layer.state = LayerState::SUM;
        break;
    case LayerState::SUM:
        layer.state = LayerState::SQSUM;
        break;
    case LayerState::SQSUM:
        layer.state = LayerState::DATA;
        break;
    }
}

void COMPUTER_VISION::verifyMatEqual(const cv::Mat& mat1, const cv::Mat& mat2, const QString& mat_name)
{
    double diff = cv::norm(mat1, mat2, cv::NORM_INF);
    if (diff != 0) {
        qDebug() << mat_name << ": X";
    }
}

void COMPUTER_VISION::verifyIntegral(int scaleIdx)
{
    const ScaleData& s = scaleData.at(scaleIdx);

    cv::Mat sbuf_sum(s.szi.height, s.szi.width, CV_32S, sbuf + s.layer_offset);
    cv::Mat sbuf_sqsum(s.szi.height, s.szi.width, CV_32S, sbuf + s.layer_offset + sqofs);
    cv::Mat rbuf_img(s.szi.height, s.szi.width, CV_8U, rbuf);
    cv::Mat sum, sqsum;

    cv::integral(rbuf_img, sum, sqsum, cv::noArray(), CV_32S, CV_32S);

    sum = sum(cv::Rect(1, 1, s.szi.width, s.szi.height));
    sqsum = sqsum(cv::Rect(1, 1, s.szi.width, s.szi.height));

# if 0:
    if (scaleIdx == 0) {
        saveMatToCsv(sbuf_sum, "my_sum.csv");
        saveMatToCsv(sbuf_sqsum, "my_sqsum.csv");
    }
    saveMatToCsv(sbuf_sum, "my_sum.csv");
    saveMatToCsv(sbuf_sqsum, "my_sqsum.csv");
    saveMatToCsv(sum, "sum.csv");
    saveMatToCsv(sqsum, "sqsum.csv");
# endif

    verifyMatEqual(sbuf_sum, sum, "sum");
    verifyMatEqual(sbuf_sqsum, sqsum, "sqsum");
}

void COMPUTER_VISION::saveMatToCsv(const cv::Mat& mat, const QString& filename)
{
    std::ofstream outputFile(filename.toStdString());
    if (!outputFile) {
        return;
    }
    outputFile << cv::format(mat, cv::Formatter::FMT_CSV);
}

void COMPUTER_VISION::displayLayer(ImgLayer& layer, int layerIndex)
{
    QImage image;
    switch (layer.state) {
    case LayerState::DATA:
    {
        cv::Mat cvImage(layer.sz.height, layer.sz.width, CV_8UC1, layer.data);
        image = QImage(cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_Grayscale8);
    }
    break;
    case LayerState::SUM:
    case LayerState::SQSUM:
    {
        cv::Mat cvImage(layer.sz.height, layer.sz.width, CV_32S, layer.state == LayerState::SUM ? layer.sum : layer.sqsum);
        image = normMat(cvImage);
    }
    break;
    }

    QLabel* label = layerLabels[layerIndex];
    label->setPixmap(QPixmap::fromImage(image.scaled(label->width(), label->height(), Qt::IgnoreAspectRatio)));
}

void COMPUTER_VISION::displayPyramid()
{
    int nLayers = imgPyramid.size();
    for (size_t i = 0; i < nLayers; ++i) {
        displayLayer(imgPyramid[i], i);
    }
}

QImage COMPUTER_VISION::normMat(cv::Mat& cvImage)
{
    double minVal, maxVal;
    cv::minMaxLoc(cvImage, &minVal, &maxVal);
    cv::Mat normalized;
    cvImage.convertTo(normalized, CV_32F, 1.0 / (maxVal - minVal), -minVal / (maxVal - minVal));
    normalized.convertTo(normalized, CV_8UC1, 255.0);

    return QImage(normalized.data, normalized.cols, normalized.rows, normalized.step, QImage::Format_Grayscale8).copy();
}
}

#endif
