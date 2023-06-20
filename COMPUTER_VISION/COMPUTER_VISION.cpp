#include "stdafx.h"
#include "COMPUTER_VISION.h"


COMPUTER_VISION::COMPUTER_VISION(QWidget* parent)
    : QMainWindow(parent),
    claheEnabled(false),
    blurEnabled(false)
{
    ui.setupUi(this);

    setData();
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
    clearImgPyramid();
    free(image);
    free(rbuf);
    free(sbuf);
}

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

void COMPUTER_VISION::initComponents()
{
    displayLabel = findChild<QLabel*>("videoLabel");
    claheBtn = findChild<QPushButton*>("claheBtn");
    blurBtn = findChild<QPushButton*>("blurBtn");
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
    initLayerLabels();
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

    timer->start(0);
}

QVector<Stage> COMPUTER_VISION::readStages(QSqlQuery& query)
{
    QVector<Stage> stages;

    if (query.exec("SELECT FIRST, NTREES, THRESHOLD FROM FRONTALFACE_DEFAULT_STAGE")) {
        while (query.next()) {
            Stage stage;
            stage.first = query.value(0).toInt();
            stage.ntrees = query.value(1).toInt();
            stage.threshold = query.value(2).toFloat();
            stages.append(stage);
        }
    }

    return stages;
}

QVector<DTree> COMPUTER_VISION::readClassifiers(QSqlQuery& query)
{
    QVector<DTree> classifiers;

    if (query.exec("SELECT NODECOUNT FROM FRONTALFACE_DEFAULT_DTREE")) {
        while (query.next()) {
            DTree classifier;
            classifier.nodeCount = query.value(0).toInt();
            classifiers.append(classifier);
        }
    }

    return classifiers;
}

QVector<DTreeNode> COMPUTER_VISION::readNodes(QSqlQuery& query)
{
    QVector<DTreeNode> nodes;

    if (query.exec("SELECT FEATUREIDX, THRESHOLD, LEFT, RIGHT FROM FRONTALFACE_DEFAULT_DTREENODE")) {
        while (query.next()) {
            DTreeNode node;
            node.featureIdx = query.value(0).toInt();
            node.threshold = query.value(1).toFloat();
            node.left = query.value(2).toInt();
            node.right = query.value(3).toInt();
            nodes.append(node);
        }
    }

    return nodes;
}

QVector<float> COMPUTER_VISION::readLeaves(QSqlQuery& query)
{
    QVector<float> leaves;

    if (query.exec("SELECT LEAVE FROM FRONTALFACE_DEFAULT_LEAVES")) {
        while (query.next()) {
            float leave = query.value(0).toFloat();
            leaves.append(leave);
        }
    }

    return leaves;
}

QVector<Stump> COMPUTER_VISION::readStumps(QSqlQuery& query)
{
    QVector<Stump> stumps;

    if (query.exec("SELECT FEATUREIDX, THRESHOLD, LEFT, RIGHT FROM FRONTALFACE_DEFAULT_STUMP")) {
        while (query.next()) {
            Stump stump;
            stump.featureIdx = query.value(0).toInt();
            stump.threshold = query.value(1).toFloat();
            stump.left = query.value(2).toFloat();
            stump.right = query.value(3).toFloat();
            stumps.append(stump);
        }
    }

    return stumps;
}

QVector<Feature> COMPUTER_VISION::readFeatures(QSqlQuery& query)
{
    QVector<Feature> features;

    QString queryString =
        "SELECT TILTED, "
        "RECT1_X, RECT1_Y, RECT1_WIDTH, RECT1_HEIGHT, RECT1_WEIGHT, "
        "RECT2_X, RECT2_Y, RECT2_WIDTH, RECT2_HEIGHT, RECT2_WEIGHT, "
        "RECT3_X, RECT3_Y, RECT3_WIDTH, RECT3_HEIGHT, RECT3_WEIGHT "
        "FROM FRONTALFACE_DEFAULT_HAAR";

    if (query.exec(queryString)) {
        while (query.next()) {
            Feature feature;
            feature.tilted = query.value(0).toBool();

            feature.rect[0].r.x = query.value(1).toInt();
            feature.rect[0].r.y = query.value(2).toInt();
            feature.rect[0].r.width = query.value(3).toInt();
            feature.rect[0].r.height = query.value(4).toInt();
            feature.rect[0].weight = query.value(5).toFloat();

            feature.rect[1].r.x = query.value(6).toInt();
            feature.rect[1].r.y = query.value(7).toInt();
            feature.rect[1].r.width = query.value(8).toInt();
            feature.rect[1].r.height = query.value(9).toInt();
            feature.rect[1].weight = query.value(10).toFloat();

            feature.rect[2].r.x = query.value(11).toInt();
            feature.rect[2].r.y = query.value(12).toInt();
            feature.rect[2].r.width = query.value(13).toInt();
            feature.rect[2].r.height = query.value(14).toInt();
            feature.rect[2].weight = query.value(15).toFloat();
            
            features.append(feature);
        }
    }

    return features;
}

bool COMPUTER_VISION::loadDataFromDB()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("HaarCascadeFeatures.db");

    if (!db.open()) {
        qDebug() << "Error: connection with database failed";
        return false;
    }

    qDebug() << "Database: connection ok";

    QSqlQuery query;

    data.stages = readStages(query);
    data.classifiers = readClassifiers(query);
    data.nodes = readNodes(query);
    data.leaves = readLeaves(query);
    data.stumps = readStumps(query);
    data.features = readFeatures(query);

    db.close();

    return true;
}

void COMPUTER_VISION::setData()
{
    data.minNodesPerTree = 1;
    data.maxNodesPerTree = 1;
    data.origWinSz = Size(24, 24);
    normrect = Rect(1, 1, data.origWinSz.width - 2, data.origWinSz.height - 2);

    if (!loadDataFromDB()) {
        qDebug() << "Error: failed to load data from database";
    }
}

void COMPUTER_VISION::calcScales()
{
    QVector<double> allScales;

    for (double factor = 1; ; factor *= 1.1f) {
        Size winSz = Size(
            doubleRound(data.origWinSz.width * factor),
            doubleRound(data.origWinSz.height * factor));
        if (winSz.width > imgSz.width || winSz.height > imgSz.height) {
            break;
        }
        allScales.append(factor);
    }

    for (size_t index = 0; index < allScales.size(); index++) {
        Size winSz = Size(
            doubleRound(data.origWinSz.width * allScales[index]),
            doubleRound(data.origWinSz.height * allScales[index]));
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

void COMPUTER_VISION::initImgProc()
{
    imgSz = Size(FRAME_W, FRAME_H);
    minObjSz = Size(30, 30);
    maxObjSz = imgSz;
    sbufSz = Size(0, 0);

    calcScales();
    updateScaleData();

    sz0 = scaleData.at(0).szi;
    sz0 = Size((int)alignSize(sz0.width, 16), sz0.height);
    {
        int nscales = scaleData.size();
        imgPyramid.resize(nscales);

        for (size_t i = 0; i < nscales; ++i) {
            imgPyramid[i].data = (unsigned char*)malloc(sizeof(unsigned char) * sz0.width * sz0.height);
            imgPyramid[i].sum = (int*)malloc(sizeof(int) * sz0.width * sz0.height);
            imgPyramid[i].sqsum = (int*)malloc(sizeof(int) * sz0.width * sz0.height);
        }
    }

    image = (unsigned char*)malloc(sizeof(unsigned char) * imgSz.width * imgSz.height);
    rbuf = (unsigned char*)malloc(sizeof(unsigned char) * sz0.width * sz0.height);
    sbuf = (int*)malloc(sizeof(int) * sbufSz.width * (sbufSz.height * 2));
    imageProcessor = new ImgProc(image, this);
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

void COMPUTER_VISION::computeChannels(int scaleIdx, unsigned char* img)
{
    const ScaleData& s = scaleData.at(scaleIdx);
    sqofs = sbufSz.area();
    integral(img, sbuf, s.szi.width, s.szi.height, s.layer_offset);
    integralSquare(img, sbuf, s.szi.width, s.szi.height, (s.layer_offset + sqofs));
# if 0:    
    verifyIntegral(scaleIdx);
# endif
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
    if (nf > 0.)
    {
        nf = std::sqrt(nf);
        int varNormFact = (float)(1. / nf);
        return area * varNormFact < 1e-1;
    }
    else
    {
        int varNormFact = 1.f;
        return false;
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
}

int COMPUTER_VISION::predictOrderedStump(int* ptr, int width, int height, double varNFact)
{
    Stump* cascadeStumps = &data.stumps[0];
    Stage* cascadeStages = &data.stages[0];
    Feature* cascadeFeatures = &data.features[0];
    int nstages = data.stages.size();
    double tmp = 0;

    for (int stageIdx = 0; stageIdx < nstages; stageIdx++) {
        Stage& stage = cascadeStages[stageIdx];
        int ntrees = stage.ntrees;
        
        tmp = 0;
        for (size_t i = 0; i < ntrees; i++) {
            Stump& stump = cascadeStumps[i];
            Feature& feature = cascadeFeatures[stump.featureIdx];
            double value = feature.calc(ptr, width, height)* varNFact;
            tmp += value < stump.threshold ? stump.left : stump.right;
        }

        if (tmp < stage.threshold) {
            return -stageIdx;
        }
        cascadeStumps += ntrees;
    }

	return 1;
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

Size COMPUTER_VISION::clacSz0(Size oriSz)
{
    int alignedSizeWidth = alignSize(oriSz.width, 16);
    return Size(alignedSizeWidth, oriSz.height);
}

void COMPUTER_VISION::calcImgPyramid()
{   
    int nscales = scaleData.size();
    QFutureWatcher<void> watcher;
    QList<QFuture<void>> futures;

    for (size_t i = 0; i < nscales; i++) {
        futures.append(QtConcurrent::run([this, i] {
            const ScaleData& s = scaleData.at(i);
            int new_w = s.szi.width;
            int new_h = s.szi.height;
            imgPyramid[i].sz.width = new_w;
            imgPyramid[i].sz.height = new_h;

            downSampling(image, imgPyramid[i].data, new_w, new_h);
            integral(imgPyramid[i].data, imgPyramid[i].sum, new_w, new_h, 0);
            integralSquare(imgPyramid[i].data, imgPyramid[i].sqsum, new_w, new_h, 0);
        }));
    }

    for (auto& future : futures) {
        watcher.setFuture(future);
        watcher.waitForFinished();
    }
}

double COMPUTER_VISION::calcNormFactor(int* pSum, int* pSqsum, int x, int y, int width)
{
    int valSum = calcAreaSum(pSum, x, y, width);
    int valSqsum = calcAreaSum(pSqsum, x, y, width);
    double nf = 576.f * valSqsum - (double)valSum * valSum;

    if (nf > 0.f) {
        nf = std::sqrt(nf);
        return (double)(1.f / nf);
    }
    else {
        return 1.f;
    }
}

void COMPUTER_VISION::calcHaarFeature()
{
    int nscales = scaleData.size(); 
    QFutureWatcher<void> watcher;
    QList<QFuture<void>> futures;
    
    for (size_t i = 4; i < nscales; i++) {
        
        futures.append(QtConcurrent::run([this, i] {
            
            const ScaleData& s = scaleData.at(i);
            double scaleFactor = s.scale;
            int* pSum = imgPyramid[i].sum;
            int* pSqsum = imgPyramid[i].sqsum;
            int width = imgPyramid[i].sz.width;
            int height = imgPyramid[i].sz.height;
            int rangeX = s.szi.width - data.origWinSz.width;
            int rangeY = s.szi.height - data.origWinSz.height;
            int step = s.ystep;
    
            for (int y = 0; y <= rangeY; y += step) {
                for (int x = 0; x <= rangeX; x += step) {
                    
                    imgPyramid[i].varNFact = calcNormFactor(pSum, pSqsum, x, y, width);
                    int result = predictOrderedStump(&pSum[y * width + x], width, height, imgPyramid[i].varNFact);
                    
                    if (result > 0) {
                        QMutexLocker locker(&facesMutex);
                        faces.append(Rect(
                            doubleRound(x * scaleFactor),
                            doubleRound(y * scaleFactor),
                            doubleRound(24 * scaleFactor),
                            doubleRound(24 * scaleFactor))
                        );
                    }
                    else if (result == 0) {
                        x += step;
                    }
                }
            }
        }));
    }  
    
    for (auto& future : futures) {
        watcher.setFuture(future);
        watcher.waitForFinished();
    }
}

bool COMPUTER_VISION::compRect(const Rect& r1, const Rect& r2) {

    double delta = 0.2 * (intMin(r1.width, r2.width) +
                          intMin(r1.height, r2.height)) * 0.5;
    return intAbs(r1.x - r2.x) <= delta &&
           intAbs(r1.y - r2.y) <= delta &&
           intAbs(r1.x + r1.width - r2.x - r2.width) <= delta &&
           intAbs(r1.y + r1.height - r2.y - r2.height) <= delta;
}

int COMPUTER_VISION::partition(const QVector<Rect>& rectList, QVector<int>& labels)
{
    int N = (int)rectList.size();
    const Rect* pRect = &rectList[0];
    const int PARENT = 0;
    const int RANK = 1;

    QVector<int> nodes(N * 2);
    int (*pNode)[2] = (int(*)[2])&nodes[0];

    for (size_t i = 0; i < N; i++) {
        pNode[i][PARENT] = -1;
        pNode[i][RANK] = 0;
    }

    for (size_t i = 0; i < N; i++) {        
        int root = i;
        while (pNode[root][PARENT] >= 0) {
            root = pNode[root][PARENT];
        }

        for (size_t j = 0; j < N; j++) {
            
            if (i == j || !compRect(rectList[i], rectList[j])) {
                continue;
            }
            
            int root2 = j;

            while (pNode[root2][PARENT] >= 0) {
                root2 = pNode[root2][PARENT];
            }

            if (root2 != root) {
                int rank = pNode[root][RANK];
                int rank2 = pNode[root2][RANK];
                if (rank > rank2) {
                    pNode[root2][PARENT] = root;
                }
                else {
                    pNode[root][PARENT] = root2;
                    pNode[root2][RANK] += rank == rank2;
                    root = root2;
                }

                int k = j;
                int parent;

                while ((parent = pNode[k][PARENT]) >= 0) {
                    pNode[k][PARENT] = root;
                    k = parent;
                }

                k = i;
                while ((parent = pNode[k][PARENT]) >= 0) {
                    pNode[k][PARENT] = root;
                    k = parent;
                }
            }
        }
    }

    labels.resize(N);
    int nclasses = 0;

    for (size_t i = 0; i < N; i++) {
        int root = i;
        while (pNode[root][PARENT] >= 0) {
			root = pNode[root][PARENT];
		}
        if (pNode[root][RANK] >= 0) {
            pNode[root][RANK] = ~nclasses++;
        }
        labels[i] = ~pNode[root][RANK];
    }

    return nclasses;
}

void COMPUTER_VISION::groupRectangles(QVector<Rect>& rectList, int threshold, double eps)
{
    if (rectList.empty()) {
        return;
    }

    QVector<int> labels;
    int nclasses = partition(rectList, labels);
    QVector<Rect> rrects(nclasses);
    QVector<int> rweights(nclasses, 0);
    int nlabels = (int)labels.size();
    int i, j;

    for (i = 0; i < nlabels; i++) {
        int cls = labels[i];
        rrects[cls].x += rectList[i].x;
        rrects[cls].y += rectList[i].y;
        rrects[cls].width += rectList[i].width;
        rrects[cls].height += rectList[i].height;
        rweights[cls]++;
    }

    for (i = 0; i < nclasses; i++) {
        Rect r = rrects[i];
        double s = 1.f / rweights[i];
        rrects[i] = Rect(doubleToInt((double)r.x * s),
            		     doubleToInt((double)r.y * s),
            		     doubleToInt((double)r.width * s),
            		     doubleToInt((double)r.height * s));
    }

    rectList.clear();

    for (i = 0; i < nclasses; i++) {
        
        Rect r1 = rrects[i];
        int n1 = rweights[i];

        if (n1 <= threshold) {
            continue;
        }

        for (j = 0; j < nclasses; j++) {
            
            int n2 = rweights[j];

            if (j == i || n2 <= threshold) {
				continue;
			}

            Rect r2 = rrects[j];
            int dx = doubleToInt((double)r2.width * eps);
            int dy = doubleToInt((double)r2.height * eps);

            if (i != j &&
                r1.x >= r2.x - dx &&
                r1.y >= r2.y - dy &&
                r1.x + r1.width <= r2.x + r2.width + dx &&
                r1.y + r1.height <= r2.y + r2.height + dy &&
                (n2 > intMax(3, n1)) || (n1 < 3)) {
                break;
            }
        }

        if (j == nclasses) {
            rectList.push_back(r1);
        }
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

void COMPUTER_VISION::clearImgPyramid()
{
    for (size_t i = 0; i < imgPyramid.size(); i++) {
        free(imgPyramid[i].data);
        free(imgPyramid[i].sum);
        free(imgPyramid[i].sqsum);
    }
    imgPyramid.clear();
}

void COMPUTER_VISION::updateFrame()
{
    acqFrame();
    procImg();
    displayImg(); 

    calcImgPyramid();
    calcHaarFeature();
    groupRectangles(faces, 10, 0.2);

    drawFaces();
}

void COMPUTER_VISION::drawFaces()
{
    if (faces.empty()) {
        return;
    }

    QPixmap pixmap = displayLabel->pixmap(); 
    QPainter painter(&pixmap);

    QPen pen;
    pen.setWidth(5);
    pen.setColor(Qt::green);
    painter.setPen(pen);

    for (int i = 0; i < faces.size(); ++i) {
        Rect face = faces[i];
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
