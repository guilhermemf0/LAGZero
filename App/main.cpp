#include "mainwindow.h"
#include "splashscreen.h"
#include <QApplication>
#include <QTimer>
#include <QMetaType>
#include <QJsonObject>
#include <QSslSocket>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<QList<QJsonObject>>("QList<QJsonObject>");

    SplashScreen splash;
    splash.show();

    MainWindow w;

    QTimer::singleShot(3000, [&](){
        splash.close();
        w.show();
    });

    return a.exec();
}
