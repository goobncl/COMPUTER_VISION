#include "stdafx.h"
#include "COMPUTER_VISION.h"


COMPUTER_VISION::COMPUTER_VISION(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    imageArray = new unsigned char[640 * 480 * 3];

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    webcamLabel = findChild<QLabel*>("videoLabel");
    grayscaleBtn = findChild<QPushButton*>("grayscaleBtn");
    isGrayscaleEnabled = false;
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &COMPUTER_VISION::updateFrame);
    connect(grayscaleBtn, &QPushButton::clicked, this, &COMPUTER_VISION::onGrayscaleBtnClicked);

    cap.open(0);
    timer->start(33);
}

COMPUTER_VISION::~COMPUTER_VISION()
{
    delete[] imageArray;
}

QImage COMPUTER_VISION::getQImageFromArray(const unsigned char* array, int width, int height)
{
    cv::Mat frameBGR(height, width, CV_8UC3, const_cast<unsigned char*>(array));
    cv::Mat frameRGB;
    cv::cvtColor(frameBGR, frameRGB, cv::COLOR_BGR2RGB);

    QImage qFrame = QImage(static_cast<const unsigned char*>(frameRGB.data), width, height, QImage::Format_RGB888).copy();
    return qFrame;
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

    memcpy(imageArray, frame.data, 640 * 480 * 3);

    QImage qFrame = getQImageFromArray(imageArray, 640, 480);
    webcamLabel->setPixmap(QPixmap::fromImage(qFrame));
}

void COMPUTER_VISION::setGrayscaleEnabled(bool isEnabled)
{
    isGrayscaleEnabled = isEnabled;

    QFont font = grayscaleBtn->font();
    if (isGrayscaleEnabled)
    {
        font.setWeight(QFont::Bold);
    }
    else
    {
        font.setWeight(QFont::Normal);
    }
    grayscaleBtn->setFont(font);
}

void COMPUTER_VISION::onGrayscaleBtnClicked()
{
    setGrayscaleEnabled(!isGrayscaleEnabled);
}
