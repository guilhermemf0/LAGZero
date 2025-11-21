#include "staticchartwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <cmath> // Para pow

StaticChartWidget::StaticChartWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void StaticChartWidget::loadData(const QList<double>& fps, const QList<double>& temp)
{
    m_fpsData = fps;
    m_tempData = temp;
    m_hasEquation = false; // Reseta equação ao carregar novos dados
    update();
}

void StaticChartWidget::setEquation(double a, double b, double c)
{
    m_eqA = a; m_eqB = b; m_eqC = c;
    m_hasEquation = true;
    update();
}

void StaticChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.fillRect(rect(), QColor("#0d1117"));
    p.setPen(QColor("#30363d"));
    p.drawRect(rect().adjusted(0,0,-1,-1));

    if (m_fpsData.isEmpty() || m_fpsData.size() != m_tempData.size()) {
        p.setPen(Qt::white);
        p.drawText(rect(), Qt::AlignCenter, "Sem dados.");
        return;
    }

    // Escalas
    double minTemp = 100, maxTemp = 0;
    double minFps = 1000, maxFps = 0;

    for (double v : m_tempData) { if(v < minTemp) minTemp = v; if(v > maxTemp) maxTemp = v; }
    for (double v : m_fpsData) { if(v < minFps) minFps = v; if(v > maxFps) maxFps = v; }

    minTemp = std::max(0.0, minTemp - 5); maxTemp += 5;
    minFps = std::max(0.0, minFps - 10); maxFps += 10;

    int leftMargin = 40; int bottomMargin = 30;
    QRect graphRect = rect().adjusted(leftMargin, 10, -10, -bottomMargin);

    // Eixos
    p.setPen(QColor("#8b949e"));
    p.drawLine(graphRect.bottomLeft(), graphRect.bottomRight());
    p.drawLine(graphRect.bottomLeft(), graphRect.topLeft());
    p.drawText(rect().adjusted(0, height()-25, 0, 0), Qt::AlignBottom|Qt::AlignHCenter, "Temperatura (°C)");

    p.save(); p.translate(15, height()/2); p.rotate(-90);
    p.drawText(0, 0, "FPS"); p.restore();

    // 1. Desenha Pontos (Scatter) - Realidade
    p.setBrush(QColor("#58a6ff")); p.setPen(Qt::NoPen);
    for (int i = 0; i < m_fpsData.size(); ++i) {
        double x = graphRect.left() + ((m_tempData[i] - minTemp) / (maxTemp - minTemp)) * graphRect.width();
        double y = graphRect.bottom() - ((m_fpsData[i] - minFps) / (maxFps - minFps)) * graphRect.height();
        p.drawEllipse(QPointF(x, y), 3, 3);
    }

    // 2. Desenha Equação (Curva) - Modelo Matemático
    if (m_hasEquation) {
        QPainterPath curvePath;
        bool firstPoint = true;

        // Desenha a curva pixel a pixel no eixo X
        for (int px = 0; px <= graphRect.width(); px += 2) {
            // Converte Pixel X -> Temperatura
            double t = minTemp + (double(px) / graphRect.width()) * (maxTemp - minTemp);

            // Aplica a Equação: FPS = a*T^2 + b*T + c
            double fpsCalculado = (m_eqA * std::pow(t, 2)) + (m_eqB * t) + (m_eqC);

            // Converte FPS -> Pixel Y
            double py = graphRect.bottom() - ((fpsCalculado - minFps) / (maxFps - minFps)) * graphRect.height();

            // Clamp para não desenhar fora do gráfico
            if (py < graphRect.top() || py > graphRect.bottom()) continue;

            if (firstPoint) { curvePath.moveTo(graphRect.left() + px, py); firstPoint = false; }
            else { curvePath.lineTo(graphRect.left() + px, py); }
        }

        p.setPen(QPen(QColor("#238636"), 2)); // Verde
        p.setBrush(Qt::NoBrush);
        p.drawPath(curvePath);

        // Legenda da Linha
        p.drawText(graphRect.topRight() + QPoint(-100, 20), "— Modelo f(x)");
    }

    // Textos Min/Max
    p.setPen(QColor("#8b949e"));
    p.drawText(graphRect.bottomLeft() + QPoint(0, 15), QString::number(minTemp, 'f', 0) + "°");
    p.drawText(graphRect.bottomRight() + QPoint(-20, 15), QString::number(maxTemp, 'f', 0) + "°");
    p.drawText(graphRect.topLeft() + QPoint(-30, 10), QString::number(maxFps, 'f', 0));
}
