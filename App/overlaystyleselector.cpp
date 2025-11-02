#include "overlaystyleselector.h"
#include <QPainter>
#include <QHBoxLayout>
#include <QDebug>

OverlayStyleSelector::OverlayStyleSelector(QWidget *parent) : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(0,0,0,0);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    auto* detailed = new OverlayStylePreview(0, this);
    auto* compact = new OverlayStylePreview(1, this);

    m_buttonGroup->addButton(detailed, 0);
    m_buttonGroup->addButton(compact, 1);

    layout->addWidget(detailed);
    layout->addWidget(compact);
}

OverlayStylePreview::OverlayStylePreview(int styleId, QWidget *parent)
    : QPushButton(parent), m_styleId(styleId)
{
    setMinimumSize(180, 100);
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setStyleSheet(R"(
        OverlayStylePreview {
            background-color: #1e293b;
            border: 2px solid #334155;
            border-radius: 8px;
            color: #94a3b8;
            font-family: 'Inter';
        }
        OverlayStylePreview:hover {
            border-color: #0085ff;
        }
        OverlayStylePreview:checked {
            border-color: #0085ff;
            background-color: #0d1e3a;
        }
    )");
}

void OverlayStylePreview::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_styleId == 0) {
        drawStyleDetailed(painter);
    } else {
        drawStyleCompact(painter);
    }
}

void OverlayStylePreview::drawStyleDetailed(QPainter &painter)
{
    painter.setFont(QFont("Inter", 8, QFont::Bold));
    painter.setPen(QColor("#00D1FF"));
    painter.drawText(10, 20, "+- LAG ZERO -------+");
    painter.drawText(10, 32, "| FPS  144 AVG 142");
    painter.setPen(QColor("#FF00FF"));
    painter.drawText(10, 44, "| CPU  50% | 65C");
    painter.setPen(QColor("#00FF00"));
    painter.drawText(10, 56, "| GPU  80% | 72C");
    painter.setPen(QColor("#FFFF00"));
    painter.drawText(10, 68, "| RAM  12345 MB");
    painter.setPen(QColor("#00D1FF"));
    painter.drawText(10, 80, "+------------------+");

    painter.setFont(QFont("Inter", 9, QFont::Bold));
    painter.setPen(isChecked() ? QColor("#FFFFFF") : QColor("#94a3b8"));
    painter.drawText(rect().adjusted(0,0,0,-10), Qt::AlignBottom | Qt::AlignHCenter, "Detalhado");
}

void OverlayStylePreview::drawStyleCompact(QPainter &painter)
{
    painter.setFont(QFont("Inter", 8, QFont::Bold));
    painter.setPen(QColor("#FFFFFF"));
    painter.drawText(10, 20, "FPS: 144");
    painter.setPen(QColor("#94a3b8"));
    painter.drawText(70, 20, "(AVG 142 | MIN 90)");

    painter.setPen(QColor("#FF00FF"));
    painter.drawText(10, 40, "CPU:");
    painter.setPen(QColor("#FFFFFF"));
    painter.drawText(40, 40, "50% | 65C | 80W");

    painter.setPen(QColor("#00FF00"));
    painter.drawText(10, 60, "GPU:");
    painter.setPen(QColor("#FFFFFF"));
    painter.drawText(40, 60, "80% | 72C | 150W");

    painter.setFont(QFont("Inter", 9, QFont::Bold));
    painter.setPen(isChecked() ? QColor("#FFFFFF") : QColor("#94a3b8"));
    painter.drawText(rect().adjusted(0,0,0,-10), Qt::AlignBottom | Qt::AlignHCenter, "Compacto");
}
