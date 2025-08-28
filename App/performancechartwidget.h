#ifndef PERFORMANCECHARTWIDGET_H
#define PERFORMANCECHARTWIDGET_H

#include <QWidget>
#include <QList>
#include <QPointF>
#include <QElapsedTimer>

// Widget customizado para desenhar um gráfico de performance (Temp vs FPS)
class PerformanceChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PerformanceChartWidget(QWidget *parent = nullptr);

    // Adiciona um novo ponto de dados ao gráfico
    void addDataPoint(double temperature, double fps);
    // Limpa todos os dados do gráfico
    void clearData();

    // --- NOVO: Funções para acessar os dados com segurança ---
    QList<double> getTempData() const;
    QList<double> getFpsData() const;


protected:
    // Evento de pintura onde o gráfico é desenhado
    void paintEvent(QPaintEvent *event) override;

private:
    QList<double> m_tempData;
    QList<double> m_fpsData;
    int m_maxDataPoints = 120; // Armazena os últimos 2 minutos de dados (120 segundos)

    // Funções auxiliares para o desenho
    void drawBackground(QPainter &painter);
    void drawGrid(QPainter &painter);
    void drawAxes(QPainter &painter, double maxTemp, double maxFps);
    void drawDataLine(QPainter &painter, const QList<double>& data, const QColor& color, double maxValue, const QRectF& plotArea);
};

#endif // PERFORMANCECHARTWIDGET_H
