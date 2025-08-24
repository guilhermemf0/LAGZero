#include "splashscreen.h"
#include "ui_splashscreen.h"
#include <QPropertyAnimation>
#include <QFontDatabase> // Necessário para adicionar fontes personalizadas
#include <QDebug>        // Necessário para verificar se a fonte foi carregada

SplashScreen::SplashScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SplashScreen)
{
    ui->setupUi(this);

    // --- Configuração da Janela ---
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // --- Estilo ---
    // Cor de fundo exata do seu site e bordas arredondadas
    this->setStyleSheet("QWidget#SplashScreen { background-color: #05070d; border-radius: 10px; }");

    // Adiciona e verifica a fonte personalizada "Audiowide"
    int fontId = QFontDatabase::addApplicationFont(":/fonts/Audiowide-Regular.ttf");
    QString fontFamily;
    if (fontId != -1) {
        fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    } else {
        // Se a fonte não for encontrada, imprime um aviso e usa uma fonte padrão
        qDebug() << "Aviso: A fonte 'Audiowide-Regular.ttf' não foi encontrada no caminho de recursos ':/fonts/'. Usando uma fonte padrão.";
        fontFamily = "sans-serif";
    }

    // Define um estilo base para ambas as partes do título, usando a fonte do site
    const QString baseTitleStyle = QString(
        "font-family: '%1';"
        "font-size: 38pt;"      // Tamanho ajustado para melhor aparência
        "font-weight: normal;"
        "padding: 0px;"         // Remove o preenchimento
        "margin: 0px;"          // Remove a margem
    ).arg(fontFamily);

    // Estiliza a parte "LAG" para ser branca
    ui->lagLabel->setStyleSheet(baseTitleStyle + "color: #ffffff;");

    // Estiliza a parte "ZERO" com o gradiente exato do site
    ui->zeroLabel->setStyleSheet(baseTitleStyle +
        "color: qlineargradient(spread:pad, x1:0, y1:0.5, x2:1, y2:0.5, "
        "stop:0 rgba(0, 133, 255, 255), "  // --color-accent-primary
        "stop:1 rgba(0, 209, 255, 255));"   // --color-accent-secondary
    );

    // --- Animação ---
    // Aumenta a duração para corresponder à do site
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(2000); // 2 segundos
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::InOutCubic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

SplashScreen::~SplashScreen()
{
    delete ui;
}
