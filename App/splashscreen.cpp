#include "splashscreen.h"
#include "ui_splashscreen.h"
#include "strokedlabel.h"
#include <QPropertyAnimation>
#include <QFontDatabase>
#include <QDebug>
#include <QPen>
#include <QLinearGradient>

SplashScreen::SplashScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SplashScreen)
{
    ui->setupUi(this);

    // --- Configuração da Janela ---
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // --- Estilo ---
    this->setStyleSheet("QWidget#SplashScreen { background-color: #05070d; border-radius: 10px; }");

    // --- Fonte ---
    int fontId = QFontDatabase::addApplicationFont(":/fonts/Audiowide-Regular.ttf");
    QString fontFamily;
    if (fontId != -1) {
        fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    } else {
        qDebug() << "Aviso: A fonte 'Audiowide-Regular.ttf' não foi encontrada. Usando uma fonte padrão.";
        fontFamily = "sans-serif";
    }

    QFont titleFont(fontFamily, 38, QFont::Normal);

    // --- Criação dos StrokedLabels ---
    m_lagLabel = new StrokedLabel("LAG", this);
    m_zeroLabel = new StrokedLabel("ZERO", this);

    // --- CORREÇÃO: Borda ainda mais espessa ---
    QPen strokePen(QColor("#000000"), 5.0); // Aumentado de 3.5 para 5.0
    strokePen.setJoinStyle(Qt::RoundJoin);

    // --- Estiliza o "LAG" ---
    m_lagLabel->setFont(titleFont);
    m_lagLabel->setStrokePen(strokePen);
    m_lagLabel->setFillBrush(QColor("#ffffff"));
    m_lagLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // --- Estiliza o "ZERO" ---
    QLinearGradient zeroGradient(0, 0, m_zeroLabel->sizeHint().width(), 0);
    zeroGradient.setColorAt(0, QColor(0, 133, 255));
    zeroGradient.setColorAt(1, QColor(0, 209, 255));

    m_zeroLabel->setFont(titleFont);
    m_zeroLabel->setStrokePen(strokePen);
    m_zeroLabel->setFillBrush(zeroGradient);
    m_zeroLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // --- CORREÇÃO: Centralização e Mais Espaçamento ---
    ui->titleLayout->addStretch(1);
    ui->titleLayout->addWidget(m_lagLabel);
    ui->titleLayout->addWidget(m_zeroLabel);
    ui->titleLayout->addStretch(1);
    ui->titleLayout->setSpacing(10); // Aumentado de 4 para 10

    // --- Animação ---
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(2000);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::InOutCubic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

SplashScreen::~SplashScreen()
{
    delete ui;
}
