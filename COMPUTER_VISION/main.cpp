#include "stdafx.h"
#include "COMPUTER_VISION.h"
#include <QtWidgets/QApplication>
#include <QFlags>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    COMPUTER_VISION w;

    Qt::WindowFlags flags = w.windowFlags();
    flags &= ~Qt::WindowMaximizeButtonHint;
    flags |= Qt::MSWindowsFixedSizeDialogHint;

    w.setWindowFlags(flags);
    w.show();
    return a.exec();
}
