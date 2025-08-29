#include "performancechartwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QMouseEvent>
#include <algorithm>

PerformanceChartWidget::PerformanceChartWidget(QWidget *parent)
    : QWidget{parent},
    m_tempColor("#FF7043"),
    m_fpsColor("#00D1FF"),
    m_tempLabel("Temperatura (°C)"),
    m_fpsLabel("FPS")
{
    setMinimumHeight(280);
    setMouseTracking(true);
}

void PerformanceChartWidget::addDataPoint(double temperature, double fps)
{
    m_tempData.append(temperature);
    m_fpsData.append(fps);

    while (m_tempData.size() > m_maxDataPoints) m_tempData.removeFirst();
    while (m_fpsData.size() > m_maxDataPoints) m_fpsData.removeFirst();

    update();
}

void PerformanceChartWidget::clearData()
{
    m_tempData.clear();
    m_fpsData.clear();
    update();
}

void PerformanceChartWidget::setColors(const QColor& tempColor, const QColor& fpsColor)
{
    m_tempColor = tempColor;
    m_fpsColor = fpsColor;
}

void PerformanceChartWidget::setLabels(const QString& tempLabel, const QString& fpsLabel)
{
    m_tempLabel = tempLabel;
    m_fpsLabel = fpsLabel;
}

// NOVO: Implementação do método para alterar o período do gráfico
void PerformanceChartWidget::setMaxDataPoints(int points)
{
    if (points > 1) {
        m_maxDataPoints = points;
    }
}


QList<double> PerformanceChartWidget::getTempData() const { return m_tempData; }
QList<double> PerformanceChartWidget::getFpsData() const { return m_fpsData; }

void PerformanceChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawBackground(painter);

    if (m_tempData.size() < 2) {
        painter.setPen(QColor("#aeb9d6"));
        painter.drawText(rect(), Qt::AlignCenter, "Aguardando dados da sessão...");
        return;
    }

    // Dividir a área de desenho em duas para os gráficos
    QRectF totalArea = rect().adjusted(10, 10, -10, -10);
    QRectF fpsArea(totalArea.left(), totalArea.top(), totalArea.width(), totalArea.height() / 2 - 5);
    QRectF tempArea(totalArea.left(), totalArea.top() + totalArea.height() / 2 + 5, totalArea.width(), totalArea.height() / 2 - 5);

    drawSingleChart(painter, fpsArea, m_fpsData, m_fpsColor, m_fpsLabel);
    drawSingleChart(painter, tempArea, m_tempData, m_tempColor, m_tempLabel);

    if (m_mouseInside) {
        drawTracker(painter);
    }
}

void PerformanceChartWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePos = event->pos();
    m_mouseInside = true;
    update();
    QWidget::mouseMoveEvent(event);
}

void PerformanceChartWidget::leaveEvent(QEvent *event)
{
    m_mouseInside = false;
    update();
    QWidget::leaveEvent(event);
}

void PerformanceChartWidget::drawBackground(QPainter &painter)
{
    QColor bgColor(16, 18, 26, 220);
    QColor borderColor(255, 255, 255, 15);
    painter.setPen(QPen(borderColor, 1.5));
    painter.setBrush(bgColor);
    painter.drawRoundedRect(this->rect(), 12, 12);
}

void PerformanceChartWidget::drawSingleChart(QPainter &painter, const QRectF& area, const QList<double>& data, const QColor& color, const QString& label)
{
    if (data.size() < 2) return;

    painter.save();

    QPen gridPen(QColor(255, 255, 255, 8), 1, Qt::DotLine);
    painter.setPen(gridPen);
    for (int i = 1; i < 4; ++i) {
        qreal y = area.top() + i * area.height() / 4;
        painter.drawLine(area.left() + 30, y, area.right() - 30, y);
    }

    QPainterPath path = createSmoothPath(data, area);

    QPainterPath fillPath = path;
    fillPath.lineTo(area.bottomRight());
    fillPath.lineTo(area.bottomLeft());
    fillPath.closeSubpath();
    QLinearGradient gradient(area.topLeft(), area.bottomLeft());
    gradient.setColorAt(0, QColor(color.red(), color.green(), color.blue(), 60));
    gradient.setColorAt(1, QColor(color.red(), color.green(), color.blue(), 0));
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawPath(fillPath);

    QPen linePen(color, 2.5);
    painter.setPen(linePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);

    double minValue = *std::min_element(data.begin(), data.end());
    double maxValue = *std::max_element(data.begin(), data.end());
    double currentValue = data.last();

    QFont labelFont("Inter", 9, QFont::Medium);
    QFont valueFont("Inter", 16, QFont::Bold);
    QFont minMaxFont("Inter", 8, QFont::Normal);

    painter.setPen(QColor("#aeb9d6"));
    painter.setFont(labelFont);
    painter.drawText(area.adjusted(15, 8, 0, 0), Qt::AlignLeft | Qt::AlignTop, label);

    painter.setFont(minMaxFont);
    painter.drawText(area.adjusted(0, 0, -15, -5), Qt::AlignRight | Qt::AlignBottom, QString("Min %1 / Máx %2").arg(minValue, 0, 'f', 0).arg(maxValue, 0, 'f', 0));

    painter.setPen(color);
    painter.setFont(valueFont);
    painter.drawText(area.adjusted(0, 8, -15, 0), Qt::AlignRight | Qt::AlignTop, QString::number(currentValue, 'f', 0));

    painter.restore();
}

void PerformanceChartWidget::drawTracker(QPainter &painter)
{
    QRectF totalArea = rect().adjusted(10, 10, -10, -10);
    if (!totalArea.contains(m_mousePos)) return;

    painter.setPen(QPen(QColor(255, 255, 255, 50), 1.5, Qt::DashLine));
    painter.drawLine(m_mousePos.x(), totalArea.top(), m_mousePos.x(), totalArea.bottom());

    int index = static_cast<int>(((m_mousePos.x() - totalArea.left()) / totalArea.width()) * m_tempData.size());
    index = std::clamp(index, 0, static_cast<int>(m_tempData.size() - 1));

    double tempValue = m_tempData.at(index);
    double fpsValue = m_fpsData.at(index);

    QFont font("Inter", 9, QFont::Bold);
    painter.setFont(font);
    QFontMetrics fm(font);

    auto drawValueBox = [&](const QString& text, const QColor& color, qreal yPos) {
        QRectF box = fm.boundingRect(text).adjusted(-5, -3, 5, 3);
        box.moveCenter(QPointF(m_mousePos.x(), yPos));
        if (box.left() < totalArea.left()) box.moveLeft(totalArea.left());
        if (box.right() > totalArea.right()) box.moveRight(totalArea.right());
        painter.setBrush(QColor("#0c0a15"));
        painter.setPen(color);
        painter.drawRoundedRect(box, 4, 4);
        painter.drawText(box, Qt::AlignCenter, text);
    };

    drawValueBox(QString::number(fpsValue, 'f', 0) + " FPS", m_fpsColor, totalArea.top() + totalArea.height() / 4);
    drawValueBox(QString::number(tempValue, 'f', 1) + " °C", m_tempColor, totalArea.bottom() - totalArea.height() / 4);
}

QPainterPath PerformanceChartWidget::createSmoothPath(const QList<double>& data, const QRectF& area)
{
    if (data.size() < 2) return QPainterPath();

    double minValue = *std::min_element(data.begin(), data.end());
    double maxValue = *std::max_element(data.begin(), data.end());
    double range = maxValue - minValue;
    if (range < 1) range = 1;

    QPainterPath path;
    QVector<QPointF> points(data.size());
    double xStep = area.width() / (data.size() - 1);

    for (int i = 0; i < data.size(); ++i) {
        double y = area.bottom() - ((data[i] - minValue) / range) * area.height();
        points[i] = QPointF(area.left() + i * xStep, y);
    }

    path.moveTo(points[0]);
    for (int i = 0; i < points.size() - 1; ++i) {
        QPointF p1 = points[i];
        QPointF p2 = points[i+1];
        QPointF ctrl1((p1.x() + p2.x()) / 2, p1.y());
        QPointF ctrl2((p1.x() + p2.x()) / 2, p2.y());
        path.cubicTo(ctrl1, ctrl2, p2);
    }
    return path;
}
