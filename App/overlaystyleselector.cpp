#include "overlaystyleselector.h"
#include <QPainter>
#include <QHBoxLayout>
#include <QDebug>
#include <QList>
#include <QColor>

QString getPreviewGradientColor(int index, int total) {
    if (total <= 1) total = 1;
    if (index > total) index = total;
    QColor colorA = QColor::fromHsl(190, 255, 128);
    QColor colorB = QColor::fromHsl(275, 204, 153);
    double ratio = 0.0;
    if (total > 1) {
        ratio = static_cast<double>(index) / static_cast<double>(total - 1);
    }
    int hue = colorA.hue() + (colorB.hue() - colorA.hue()) * ratio;
    int sat = colorA.saturation() + (colorB.saturation() - colorA.saturation()) * ratio;
    int lig = colorA.lightness() + (colorB.lightness() - colorA.lightness()) * ratio;
    QColor finalColor = QColor::fromHsl(hue, sat, lig);
    return finalColor.name().toUpper().remove('#');
}


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
    setMinimumSize(180, 340);
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setStyleSheet(R"(
        OverlayStylePreview {
            background-color: #1e293b;
            border: 2px solid #334155;
            border-radius: 8px;
            color: #94a3b8;
            font-family: 'Inter';
            text-align: left;
            padding: 10px;
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

void OverlayStylePreview::drawMetric(QPainter &painter, QFontMetrics &fm, int &y, const QColor& color, const QString& label, const QString& value, const QString& unit)
{
    int rightAlign = 160;
    int labelWidth = 10;
    int valueWidth = 5;

    painter.setPen(color);
    painter.drawText(10, y, label.leftJustified(labelWidth));
    painter.setPen(QColor("#FFFFFF"));
    QString text = QString("%1 %2").arg(value.rightJustified(valueWidth), unit.leftJustified(4));
    painter.drawText(rightAlign - fm.horizontalAdvance(text), y, text);
    y += 12;
}


void OverlayStylePreview::drawStyleDetailed(QPainter &painter)
{
    painter.setFont(QFont("Inter", 8, QFont::Bold));

    QFontMetrics fm = painter.fontMetrics();
    int y = 20;
    int lineIndex = 0;
    const int totalLines = 16;

    auto getNextColor = [&]() {
        return QColor(QString("#%1").arg(getPreviewGradientColor(lineIndex++, totalLines)));
    };

    painter.setPen(QColor("#00D1FF"));
    painter.drawText(10, y, "LAG ZERO");
    y += 17;

    drawMetric(painter, fm, y, getNextColor(), "FPS", "144", "");
    drawMetric(painter, fm, y, getNextColor(), "AVG", "142", "");
    drawMetric(painter, fm, y, getNextColor(), "MIN", "60", "");

    y += 10;

    painter.setPen(getNextColor());
    painter.drawText(10, y, "AMD Ryzen 5 5500");
    y += 12;

    drawMetric(painter, fm, y, getNextColor(), "TEMP", "65", "°C");
    drawMetric(painter, fm, y, getNextColor(), "USO", "50", "%");
    drawMetric(painter, fm, y, getNextColor(), "PWR", "80", "W");
    drawMetric(painter, fm, y, getNextColor(), "CLK", "4200", "MHz");

    y += 10;

    painter.setPen(getNextColor());
    painter.drawText(10, y, "Radeon RX 6600");
    y += 12;

    drawMetric(painter, fm, y, getNextColor(), "TEMP", "72", "°C");
    drawMetric(painter, fm, y, getNextColor(), "USO", "99", "%");
    drawMetric(painter, fm, y, getNextColor(), "PWR", "100", "W");
    drawMetric(painter, fm, y, getNextColor(), "CLK", "2600", "MHz");

    y += 10;

    painter.setPen(getNextColor());
    painter.drawText(10, y, "OUTROS");
    y += 12;

    drawMetric(painter, fm, y, getNextColor(), "RAM", "12800", "MB");
    drawMetric(painter, fm, y, getNextColor(), "PLACA MAE", "55", "°C");
    drawMetric(painter, fm, y, getNextColor(), "SSD", "45", "°C");

    y += 10;

    drawMetric(painter, fm, y, getNextColor(), "CPU", "1400", "RPM");
    drawMetric(painter, fm, y, getNextColor(), "GPU", "2100", "RPM");


    painter.setFont(QFont("Inter", 9, QFont::Bold));
    painter.setPen(isChecked() ? QColor("#FFFFFF") : QColor("#94a3b8"));
    painter.drawText(rect().adjusted(0,0,0,-10), Qt::AlignBottom | Qt::AlignHCenter, "Detalhado");
}

void OverlayStylePreview::drawStyleCompact(QPainter &painter)
{
    painter.setFont(QFont("Inter", 8, QFont::Bold));

    QFontMetrics fm = painter.fontMetrics();
    int y = 20;
    int lineIndex = 0;
    const int totalLines = 14;

    auto getNextColor = [&]() {
        return QColor(QString("#%1").arg(getPreviewGradientColor(lineIndex++, totalLines)));
    };

    drawMetric(painter, fm, y, getNextColor(), "FPS", "144", "");
    drawMetric(painter, fm, y, getNextColor(), "AVG", "142", "");
    drawMetric(painter, fm, y, getNextColor(), "MIN", "60", "");

    y += 5;

    drawMetric(painter, fm, y, getNextColor(), "CPU T", "65", "°C");
    drawMetric(painter, fm, y, getNextColor(), "CPU U", "50", "%");
    drawMetric(painter, fm, y, getNextColor(), "PWR", "80", "W");
    drawMetric(painter, fm, y, getNextColor(), "CLK", "4200", "MHz");

    y += 5;

    drawMetric(painter, fm, y, getNextColor(), "GPU T", "72", "°C");
    drawMetric(painter, fm, y, getNextColor(), "GPU U", "99", "%");
    drawMetric(painter, fm, y, getNextColor(), "PWR", "100", "W");
    drawMetric(painter, fm, y, getNextColor(), "CLK", "2600", "MHz");

    y += 5;

    drawMetric(painter, fm, y, getNextColor(), "RAM", "12800", "MB");
    drawMetric(painter, fm, y, getNextColor(), "PLACA MAE", "55", "°C");
    drawMetric(painter, fm, y, getNextColor(), "SSD", "45", "°C");

    y += 5;

    drawMetric(painter, fm, y, getNextColor(), "CPU", "1400", "RPM");
    drawMetric(painter, fm, y, getNextColor(), "GPU", "2100", "RPM");

    painter.setFont(QFont("Inter", 9, QFont::Bold));
    painter.setPen(isChecked() ? QColor("#FFFFFF") : QColor("#94a3b8"));
    painter.drawText(rect().adjusted(0,0,0,-10), Qt::AlignBottom | Qt::AlignHCenter, "Simples");
}
