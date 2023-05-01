#include "stdafx.h"
#include "COMPUTER_VISION.h"


COMPUTER_VISION::COMPUTER_VISION(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    imageArray = new unsigned char[640 * 480];

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

void COMPUTER_VISION::updateFrame()
{
    cv::Mat frame;
    cap >> frame;

    if (frame.empty())
    {
        timer->stop();
        return;
    }

    cv::Mat grayFrame;
    cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

    memcpy(imageArray, grayFrame.data, 640 * 480);

    QImage qFrame = QImage(static_cast<const unsigned char*>(grayFrame.data), 640, 480, QImage::Format_Grayscale8).copy();
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
