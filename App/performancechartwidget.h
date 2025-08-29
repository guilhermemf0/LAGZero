#ifndef PERFORMANCECHARTWIDGET_H
#define PERFORMANCECHARTWIDGET_H

#include <QWidget>
#include <QList>
#include <QColor>

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
    // NOVO: Permite alterar o período do gráfico dinamicamente
    void setMaxDataPoints(int points);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    // Dados
    QList<double> m_tempData;
    QList<double> m_fpsData;
    // ALTERADO: Valor padrão inicial
    int m_maxDataPoints = 120;

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
