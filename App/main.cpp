#include "mainwindow.h"

#include <QApplication>

#include <QSplashScreen>
#include <QPixmap>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPixmap pixmap(":/logo-site.png");
    QSplashScreen splash(pixmap);
    splash.show();

    QThread::sleep(3);

    MainWindow w;
    splash.finish(&w);
    w.show();

    return a.exec();
}
