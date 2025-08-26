#include "gamecoverwidget.h"
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

GameCoverWidget::GameCoverWidget(const QString& gameName, const QPixmap& cover, QWidget *parent)
    : QWidget{parent}
{
    // Espaço extra para a sombra e animação
    setFixedSize(150, 225);
    setCursor(Qt::PointingHandCursor);

    // Contêiner que será animado
    m_container = new QWidget(this);
    m_container->setFixedSize(130, 195);
    m_container->move(10, 15); // Centraliza com margem

    // Efeito de sombra no contêiner
    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 120));
    shadow->setOffset(0, 5);
    m_container->setGraphicsEffect(shadow);

    // Label da capa dentro do contêiner
    m_coverLabel = new QLabel(m_container);
    m_coverLabel->setGeometry(m_container->rect());
    m_coverLabel->setScaledContents(true);
    if (!cover.isNull()) {
        m_coverLabel->setPixmap(cover);
    }
    m_coverLabel->setStyleSheet("background-color: #111; border-radius: 8px; border: 1px solid rgba(255, 255, 255, 0.1);");

    // Label do nome (sobre a capa)
    m_nameLabel = new QLabel(gameName, m_coverLabel);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 0.75);
        color: white;
        font-size: 14px;
        font-weight: bold;
        padding: 5px;
        border-bottom-left-radius: 8px;
        border-bottom-right-radius: 8px;
        border-top: 1px solid rgba(255, 255, 255, 0.1);
    )");
    m_nameLabel->hide();

    // Configuração da animação "lift"
    m_animation = new QPropertyAnimation(m_container, "pos", this);
    m_animation->setDuration(150);
    m_animation->setEasingCurve(QEasingCurve::OutQuad);
}

GameCoverWidget::~GameCoverWidget() {}

void GameCoverWidget::enterEvent(QEnterEvent *event)
{
    m_nameLabel->setGeometry(0, m_coverLabel->height() - m_nameLabel->sizeHint().height(), m_coverLabel->width(), m_nameLabel->sizeHint().height());
    m_nameLabel->show();

    // Animação para "levantar" o widget
    m_animation->setStartValue(m_container->pos());
    m_animation->setEndValue(QPoint(10, 5)); // Move para cima
    m_animation->start();

    QWidget::enterEvent(event);
}

void GameCoverWidget::leaveEvent(QEvent *event)
{
    m_nameLabel->hide();

    // Animação para "abaixar" o widget
    m_animation->setStartValue(m_container->pos());
    m_animation->setEndValue(QPoint(10, 15)); // Volta à posição original
    m_animation->start();

    QWidget::leaveEvent(event);
}
