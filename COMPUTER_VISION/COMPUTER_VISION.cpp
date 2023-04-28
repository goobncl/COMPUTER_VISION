#include "stdafx.h"
#include "COMPUTER_VISION.h"


COMPUTER_VISION::COMPUTER_VISION(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    webcamLabel = findChild<QLabel*>("videoLabel");
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &COMPUTER_VISION::updateFrame);

    cap.open(1);
    timer->start(33);

}

COMPUTER_VISION::~COMPUTER_VISION()
{}

void COMPUTER_VISION::updateFrame()
{
    cv::Mat frame;
    cap >> frame;

    if (frame.empty())
    {
        timer->stop();
        return;
    }

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage qFrame((uchar*)frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);

    webcamLabel->setPixmap(QPixmap::fromImage(qFrame));
}
