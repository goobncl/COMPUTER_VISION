#include "stdafx.h"
#include "COMPUTER_VISION.h"


COMPUTER_VISION::COMPUTER_VISION(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    imageArray = new unsigned char[640 * 480];

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    displayLabel = findChild<QLabel*>("videoLabel");
    claheBtn = findChild<QPushButton*>("claheBtn");
    isGrayscaleEnabled = false;
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &COMPUTER_VISION::updateFrame);
    connect(claheBtn, &QPushButton::clicked, this, &COMPUTER_VISION::onClaheBtnClicked);

    cap.open(0);
    timer->start(0);
}

COMPUTER_VISION::~COMPUTER_VISION()
{
    delete[] imageArray;
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
    memcpy(imageArray, grayFrame.data, 640 * 480);

    // TODO: image processing with imageArray


    // Image Display
    QImage qFrame = QImage(imageArray, 640, 480, QImage::Format_Grayscale8).copy();
    displayLabel->setPixmap(QPixmap::fromImage(qFrame));

    QString fpsString = QString::number(getFPS(), 'f', 2);
    statusBar()->showMessage("FPS: " + fpsString);
}


void COMPUTER_VISION::setGrayscaleEnabled(bool isEnabled)
{
    isGrayscaleEnabled = isEnabled;

    QFont font = claheBtn->font();
    if (isGrayscaleEnabled)
    {
        font.setWeight(QFont::Bold);
    }
    else
    {
        font.setWeight(QFont::Normal);
    }
    claheBtn->setFont(font);
}

void COMPUTER_VISION::onClaheBtnClicked()
{
    setGrayscaleEnabled(!isGrayscaleEnabled);
}
