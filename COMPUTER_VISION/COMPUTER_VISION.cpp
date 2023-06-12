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
    confCap();
    setConn();
}

COMPUTER_VISION::~COMPUTER_VISION()
{
    free(image);
    free(rbuf);
    free(sbuf);
    delete timer;
    delete imageProcessor;
    cap.release();
}

void COMPUTER_VISION::initComponents()
{
    displayLabel = findChild<QLabel*>("videoLabel");
    claheBtn = findChild<QPushButton*>("claheBtn");
    blurBtn = findChild<QPushButton*>("blurBtn");
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

void COMPUTER_VISION::initLayerLabels()
{
    int nLayers = scales.size();
    for (int i = 0; i < nLayers; ++i) {
        QString objectName = "layerLabel_" + QString::number(i).rightJustified(2, '0');
        QLabel* label = findChild<QLabel*>(objectName);

        if (label) {
            layerLabels.push_back(label);
            QLabel* numLabel = createNumLabel(label, i);
            QGraphicsDropShadowEffect* effect = createDropShadowEffect();
            numLabel->setGraphicsEffect(effect);
        }
    }
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

void COMPUTER_VISION::confCap()
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
    varianceNormFactor = 0.f;

    calcScales();
    updateScaleData();
    computeOptFeatures();

    sz0 = scaleData.at(0).szi;
    sz0 = Size((int)alignSize(sz0.width, 16), sz0.height);

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
#if 0:
    {
        size_t nscales = scaleData.size();
        memset(sbuf, 0, sizeof(int) * sbufSz.width * (sbufSz.height * 2));

        for (size_t i = 0; i < nscales; i++) {
            const ScaleData& s = scaleData.at(i);
            memset(rbuf, 0, sizeof(unsigned char) * sz0.width * sz0.height);
            downSampling(image, rbuf, s.szi.width, s.szi.height);
            computeChannels(i, rbuf);
        }
    }

    {
        cv::Mat sbufMat(sbufSz.height * 2, sbufSz.width, CV_32S, sbuf);
        saveMatToCsv(sbufMat, "sbuf_raw.csv");
    }
    
    // TODO: ... 
    {
        size_t nscales = scaleData.size();
        Size origWinSz = data.origWinSz;
        
        for (size_t scaleIdx = 0; scaleIdx < nscales; scaleIdx++) {
            const ScaleData& s = scaleData.at(scaleIdx);
            int layer_offset = s.layer_offset;
            double scaleFactor = s.scale;
            int yStep = s.ystep;
            Size szw = s.getWorkingSize(origWinSz);

            for (int y = 0; y < szw.height; y += yStep) {
                for (int x = 0; x < szw.width; x += yStep) {
                    int* ptr = &sbuf[s.layer_offset + y * s.szi.width + x];
                    if (setWindow(ptr, scaleIdx)) {
                        int result = predictOrderedStump(ptr, layer_offset);
                        if (result > 0) {
                            printf("");
                        }
                        else if (result == 0) {
                            x += yStep;
                        }
                    }
                }
            }
        }
    }
#endif
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
        varianceNormFactor = (float)(1. / nf);
        return area * varianceNormFactor < 1e-1;
    }
    else
    {
        varianceNormFactor = 1.f;
        return false;
    }
}

int COMPUTER_VISION::predictOrderedStump(const int* ptr, int layer_offset)
{
    Stump* cascadeStumps = &data.stumps[0];
    Stage* cascadeStages = &data.stages[0];
    OptFeature* cascadeFeatures = &data.optFeatures[0];
    int nstages = data.stages.size();
    double tmp = 0;

    for (int stageIdx = 0; stageIdx < nstages; stageIdx++) {
        Stage& stage = cascadeStages[stageIdx];
        int ntrees = stage.ntrees;
        
        tmp = 0;
        for (int i = 0; i < ntrees; i++) {
            Stump& stump = cascadeStumps[i];
            OptFeature& feature = cascadeFeatures[stump.featureIdx];
            double value = feature.calc(ptr + layer_offset) * varianceNormFactor;
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

void COMPUTER_VISION::updateFrame()
{
    acqFrame();
    procImg();
    displayImg(); 
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
