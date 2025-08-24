#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>

// Forward declaration da nossa nova classe
class StrokedLabel;

namespace Ui {
class SplashScreen;
}

class SplashScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SplashScreen(QWidget *parent = nullptr);
    ~SplashScreen();

private:
    Ui::SplashScreen *ui;
    // Ponteiros para nossos novos widgets de texto
    StrokedLabel *m_lagLabel;
    StrokedLabel *m_zeroLabel;
};

#endif // SPLASHSCREEN_H
