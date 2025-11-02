#include "overlaypositionselector.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

OverlayPositionSelector::OverlayPositionSelector(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(100, 70);
    setMaximumSize(100, 70);
    setCursor(Qt::PointingHandCursor);
    updateRects();
}

void OverlayPositionSelector::setCurrentPosition(int id)
{
    if (id >= 0 && id < 4) {
        m_currentPosition = id;
        update();
    }
}

void OverlayPositionSelector::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(QColor("#334155"), 2));
    painter.setBrush(QColor("#1e293b"));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);

    for (int i = 0; i < 4; ++i) {
        if (i == m_currentPosition) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#0085ff"));
        } else {
            painter.setPen(QPen(QColor("#334155"), 1));
            painter.setBrush(Qt::NoBrush);
        }
        painter.drawRoundedRect(m_rects[i], 4, 4);
    }
}

void OverlayPositionSelector::mousePressEvent(QMouseEvent *event)
{
    for (int i = 0; i < 4; ++i) {
        if (m_rects[i].contains(event->pos())) {
            setCurrentPosition(i);
            emit positionSelected(i);
            break;
        }
    }
    QWidget::mousePressEvent(event);
}

void OverlayPositionSelector::resizeEvent(QResizeEvent *event)
{
    updateRects();
    QWidget::resizeEvent(event);
}

void OverlayPositionSelector::updateRects()
{
    int padding = 10;
    int cornerSize = 20;
    m_rects[0] = QRectF(padding, padding, cornerSize, cornerSize); // Top-Left
    m_rects[1] = QRectF(width() - cornerSize - padding, padding, cornerSize, cornerSize); // Top-Right
    m_rects[2] = QRectF(padding, height() - cornerSize - padding, cornerSize, cornerSize); // Bottom-Left
    m_rects[3] = QRectF(width() - cornerSize - padding, height() - cornerSize - padding, cornerSize, cornerSize); // Bottom-Right
}
