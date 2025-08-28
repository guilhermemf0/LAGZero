#ifndef PERFORMANCECHARTWIDGET_H
#define PERFORMANCECHARTWIDGET_H

#include <QWidget>
#include <QList>
#include <QColor>

// Widget de gráfico completamente redesenhado para uma visualização de dados superior.
// Agora apresenta dois gráficos empilhados (FPS e Temp) e um marcador interativo que segue o mouse.
class PerformanceChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PerformanceChartWidget(QWidget *parent = nullptr);

    void addDataPoint(double temperature, double fps);
    void clearData();
    QList<double> getTempData() const;
    QList<double> getFpsData() const;

    void setColors(const QColor& tempColor, const QColor& fpsColor);
    void setLabels(const QString& tempLabel, const QString& fpsLabel);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    // Dados
    QList<double> m_tempData;
    QList<double> m_fpsData;
    int m_maxDataPoints = 120; // 2 minutos de dados

    // Estilo
    QColor m_tempColor;
    QColor m_fpsColor;
    QString m_tempLabel;
    QString m_fpsLabel;

    // Interatividade
    bool m_mouseInside = false;
    QPointF m_mousePos;

    // Funções auxiliares de desenho
    void drawBackground(QPainter &painter);
    void drawSingleChart(QPainter &painter, const QRectF& area, const QList<double>& data, const QColor& color, const QString& label);
    void drawTracker(QPainter &painter);
    QPainterPath createSmoothPath(const QList<double>& data, const QRectF& area);
};

#endif // PERFORMANCECHARTWIDGET_H
