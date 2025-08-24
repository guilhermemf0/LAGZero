#include "mainwindow.h"
#include "splashscreen.h"
#include <QApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    SplashScreen splash;
    splash.show();

    MainWindow w;

    QTimer::singleShot(3000, [&](){
        splash.close();
        w.show();
    });

    return a.exec();
}
