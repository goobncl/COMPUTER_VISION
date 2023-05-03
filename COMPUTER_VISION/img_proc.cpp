#include "stdafx.h"
#include "img_proc.h"

void invertColors(unsigned char* input, int width, int height) {

	// just for debugging: start
	qDebug() << "[1] -*- Start on thread ID:" << QThread::currentThreadId();

	// actual image processing
	for (int i = 0; i < width * height; ++i) {
		input[i] = 255 - input[i];
	}

	// just for debugging: Stop
	qDebug() << "[2] @*@ Stop on thread ID:" << QThread::currentThreadId();
}
