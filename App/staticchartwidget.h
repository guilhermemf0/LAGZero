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

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<double> m_fpsData;
    QList<double> m_tempData;
    double m_eqA = 0, m_eqB = 0, m_eqC = 0;
    bool m_hasEquation = false;
};

#endif // STATICCHARTWIDGET_H
