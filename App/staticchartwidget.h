#ifndef STATICCHARTWIDGET_H
#define STATICCHARTWIDGET_H

#include <QWidget>
#include <QList>

class StaticChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StaticChartWidget(QWidget *parent = nullptr);
    void loadData(const QList<double>& fps, const QList<double>& temp);
    void setEquation(double a, double b, double c);
    void setTangent(double x, double y, double slope);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<double> m_fpsData;
    QList<double> m_tempData;
    double m_eqA = 0, m_eqB = 0, m_eqC = 0;
    bool m_hasEquation = false;
    double m_tanX = 0, m_tanY = 0, m_tanM = 0;
    bool m_showTangent = false;
};

#endif // STATICCHARTWIDGET_H
