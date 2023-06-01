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
    free(imageArray);
    delete timer;
    delete imageProcessor;
    cap.release();
}

void COMPUTER_VISION::initComps()
{
    displayLabel = findChild<QLabel*>("videoLabel");
    claheBtn = findChild<QPushButton*>("claheBtn");
    blurBtn = findChild<QPushButton*>("blurBtn");

    int nLayers = scales.size();
    for (int i = 0; i < nLayers; ++i) {
        QString objectName = "layerLabel_" + QString::number(i).rightJustified(2, '0');
        QLabel* label = findChild<QLabel*>(objectName);
        
        if (label) {
            layerLabels.push_back(label);
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

            QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
            effect->setBlurRadius(2);
            effect->setXOffset(1);
            effect->setYOffset(1);
            effect->setColor(QColor("black"));

            numLabel->setGraphicsEffect(effect);
        }
    }
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

    db.close();

    return true;
}

void COMPUTER_VISION::setData()
{
    data.minNodesPerTree = 1;
    data.maxNodesPerTree = 1;
    data.origWinSz = Size(24, 24);

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

            unsigned char* resized = downSampling(imageArray, new_w, new_h);

            imgPyramid[i].sz.width = new_w;
            imgPyramid[i].sz.height = new_h;
            imgPyramid[i].data = resized;
            imgPyramid[i].sum = integral(resized, new_w, new_h);
            imgPyramid[i].sqsum = integralSquare(resized, new_w, new_h);
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
        free(imgPyramid[i].sum);
        free(imgPyramid[i].sqsum);
	}
    imgPyramid.clear();
}

void COMPUTER_VISION::initImgProc()
{
    imgSz = Size(FRAME_W, FRAME_H);
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
            imageArray, 
            FRAME_W, 
            FRAME_H, 
            ImgProc::AlgType::Clahe
        );
    }   

    if (blurEnabled) {
        imageProcessor->setImageAndProcess(
            imageArray, 
            FRAME_W, 
            FRAME_H, 
            ImgProc::AlgType::Blur
        );
    }
}

void COMPUTER_VISION::displayImg()
{
    cv::Mat cvFrame;
    {
        QMutexLocker locker(&imageProcessor->mutex);
        cvFrame = cv::Mat(FRAME_H, FRAME_W, CV_8UC1, imageArray);
    }
    QImage qFrame(cvFrame.data, cvFrame.cols, cvFrame.rows, cvFrame.step, QImage::Format_Grayscale8);
    displayLabel->setPixmap(QPixmap::fromImage(std::move(qFrame)));

    statusBar()->showMessage(QString("FPS: %1").arg(getFPS(), 0, 'f', 8));
}

void COMPUTER_VISION::displayPyramid() 
{
    int nLayers = imgPyramid.size();
    for (size_t i = 0; i < nLayers; ++i) {
        cv::Mat cvImage(imgPyramid[i].sz.height, imgPyramid[i].sz.width, CV_8UC1, imgPyramid[i].data);
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
