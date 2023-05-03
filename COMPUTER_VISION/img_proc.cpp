#include "stdafx.h"
#include "img_proc.h"

void invertColors(unsigned char* input, int width, int height)
{
	qDebug() << "[1] -*- Start on thread ID:" << QThread::currentThreadId();

	for (int i = 0; i < width * height; ++i)
	{
		input[i] = 255 - input[i];
	}

	qDebug() << "[2] @*@ Finish on thread ID:" << QThread::currentThreadId();
}
