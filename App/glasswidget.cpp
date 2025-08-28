#include "glasswidget.h"
#include <QPainter>
#include <QRadialGradient>
#include <QMouseEvent>

GlassWidget::GlassWidget(QWidget *parent)
    : QWidget{parent}
{
    setMouseTracking(true);
    setAttribute(Qt::WA_TranslucentBackground);
}

void GlassWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor glassColor(15, 23, 42, 128);
    QColor borderColor(255, 255, 255, 25);

    painter.setPen(QPen(borderColor, 1));
    painter.setBrush(glassColor);
    painter.drawRoundedRect(this->rect(), 16, 16);

    if (m_mouseInside) {
        QRadialGradient gradient(m_mousePos, width() / 3.0);
        gradient.setColorAt(0, QColor(0, 133, 255, 30));
        gradient.setColorAt(0.5, QColor(0, 209, 255, 15));
        gradient.setColorAt(1, Qt::transparent);

        painter.setBrush(gradient);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(this->rect(), 16, 16);
    }
}

void GlassWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePos = event->pos();
    update();
    QWidget::mouseMoveEvent(event);
}

void GlassWidget::leaveEvent(QEvent *event)
{
    m_mouseInside = false;
    update();
    QWidget::leaveEvent(event);
}

// CORREÇÃO: A implementação agora corresponde à declaração no .h
void GlassWidget::enterEvent(QEnterEvent *event)
{
    m_mouseInside = true;
    update(); // Pede um redesenho para mostrar o brilho imediatamente
    QWidget::enterEvent(event);
}
