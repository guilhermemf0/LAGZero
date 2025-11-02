#ifndef HARDWARESUMMARYCARD_H
#define HARDWARESUMMARYCARD_H

#include <QWidget>
#include <QLabel>
#include "hardwaremonitor.h"

class HardwareSummaryCard : public QWidget
{
    Q_OBJECT
public:
    explicit HardwareSummaryCard(const QString &iconSvg, const QString &title, QWidget *parent = nullptr);
    void updateMetrics(const HardwareInfo &info);

private:
    QWidget* createMetricWidget(const QString &iconSvg, const QString &name, QLabel* &valueLabel);

    QLabel *m_mainIconLabel;
    QLabel *m_titleLabel;

    QLabel *m_tempValueLabel;
    QLabel *m_usageValueLabel;
    QLabel *m_powerValueLabel;
    QLabel *m_clockValueLabel;
};

#endif // HARDWARESUMMARYCARD_H
