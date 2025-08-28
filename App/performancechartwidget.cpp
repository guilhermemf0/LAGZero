#include "performancechartwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QFontMetrics>
#include <QLinearGradient>
#include <algorithm>

PerformanceChartWidget::PerformanceChartWidget(QWidget *parent)
    : QWidget{parent}
{
    // Aumenta a altura mínima para o gráfico ser maior
    setMinimumHeight(250);
}

void PerformanceChartWidget::addDataPoint(double temperature, double fps)
{
    m_tempData.append(temperature);
    m_fpsData.append(fps);

    while (m_tempData.size() > m_maxDataPoints) {
        m_tempData.removeFirst();
    }
    while (m_fpsData.size() > m_maxDataPoints) {
        m_fpsData.removeFirst();
    }

    update();
}

void PerformanceChartWidget::clearData()
{
    m_tempData.clear();
    m_fpsData.clear();
    update();
}

QList<double> PerformanceChartWidget::getTempData() const
{
    return m_tempData;
}

QList<double> PerformanceChartWidget::getFpsData() const
{
    return m_fpsData;
}


void PerformanceChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawBackground(painter);

    if (m_tempData.isEmpty() || m_fpsData.isEmpty()) {
        painter.setPen(QColor("#aeb9d6"));
        painter.drawText(rect(), Qt::AlignCenter, "Aguardando dados do jogo...");
        return;
    }

    double maxTemp = m_tempData.isEmpty() ? 100.0 : *std::max_element(m_tempData.begin(), m_tempData.end());
    double maxFps = m_fpsData.isEmpty() ? 60.0 : *std::max_element(m_fpsData.begin(), m_fpsData.end());
    maxTemp = std::max(100.0, maxTemp);
    maxFps = std::max(60.0, maxFps);

    drawGrid(painter);
    drawAxes(painter, maxTemp, maxFps);

    QRectF plotArea = rect().adjusted(50, 20, -20, -30);

    // Desenha as linhas de dados com preenchimento
    drawDataLine(painter, m_fpsData, QColor(0, 209, 255), maxFps, plotArea);
    drawDataLine(painter, m_tempData, QColor(255, 112, 67), maxTemp, plotArea);
}

void PerformanceChartWidget::drawBackground(QPainter &painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(16, 18, 26, 180));
    painter.drawRoundedRect(rect(), 8, 8);
}

void PerformanceChartWidget::drawGrid(QPainter &painter)
{
    QPen gridPen(QColor(255, 255, 255, 20), 1, Qt::DotLine);
    painter.setPen(gridPen);

    for (int i = 1; i < 5; ++i) {
        int x = rect().left() + 50 + i * (rect().width() - 70) / 5;
        painter.drawLine(x, rect().top() + 20, x, rect().bottom() - 30);
    }
    for (int i = 1; i < 4; ++i) {
        int y = rect().top() + 20 + i * (rect().height() - 50) / 4;
        painter.drawLine(rect().left() + 50, y, rect().right() - 20, y);
    }
}

void PerformanceChartWidget::drawAxes(QPainter &painter, double maxTemp, double maxFps)
{
    painter.setPen(QColor("#aeb9d6"));
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    painter.drawText(QRect(0, 15, 45, 15), Qt::AlignRight, "°C");
    for (int i = 0; i <= 4; ++i) {
        int y = rect().bottom() - 30 - i * (rect().height() - 50) / 4;
        double tempLabel = (maxTemp / 4.0) * i;
        painter.drawText(QRect(0, y - 10, 45, 20), Qt::AlignRight, QString::number(tempLabel, 'f', 0));
    }

    painter.drawText(QRect(rect().width() - 25, 15, 20, 15), Qt::AlignRight, "FPS");
    for (int i = 0; i <= 4; ++i) {
        int y = rect().bottom() - 30 - i * (rect().height() - 50) / 4;
        double fpsLabel = (maxFps / 4.0) * i;
        painter.drawText(QRect(rect().width() - 25, y - 10, 20, 20), Qt::AlignRight, QString::number(fpsLabel, 'f', 0));
    }

    painter.drawText(QRect(50, rect().bottom() - 20, 100, 20), Qt::AlignLeft, "Tempo (Últimos 2 min)");
}

void PerformanceChartWidget::drawDataLine(QPainter &painter, const QList<double>& data, const QColor& color, double maxValue, const QRectF& plotArea)
{
    if (data.size() < 2) return;

    QPainterPath path;
    QPainterPath fillPath;
    double xStep = plotArea.width() / (m_maxDataPoints - 1);

    double startX = plotArea.left() + (m_maxDataPoints - data.size()) * xStep;
    fillPath.moveTo(startX, plotArea.bottom());

    for (int i = 0; i < data.size(); ++i) {
        double x = startX + i * xStep;
        double y = plotArea.bottom() - (data[i] / maxValue) * plotArea.height();
        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
        fillPath.lineTo(x, y);
    }
    fillPath.lineTo(plotArea.right(), plotArea.bottom());
    fillPath.closeSubpath();

    // Desenha o preenchimento com gradiente
    QLinearGradient gradient(plotArea.topLeft(), plotArea.bottomLeft());
    gradient.setColorAt(0, color.lighter(110));
    gradient.setColorAt(1, QColor(color.red(), color.green(), color.blue(), 0));
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawPath(fillPath);

    // Desenha a linha principal
    QPen linePen(color, 2.0);
    painter.setPen(linePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
}
