#include "hardwaresummarycard.h"
#include "appconstants.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QtSvg/QSvgRenderer>
#include <QPainter>

HardwareSummaryCard::HardwareSummaryCard(const QString &iconSvg, const QString &title, QWidget *parent)
    : QWidget{parent}
{
    this->setObjectName("infoCard");
    this->setMinimumHeight(180);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 15, 20, 15);
    mainLayout->setSpacing(15);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(15);

    m_mainIconLabel = new QLabel(this);
    m_mainIconLabel->setFixedSize(32, 32);
    QSvgRenderer renderer;
    renderer.load(iconSvg.toUtf8());
    QPixmap pixmap(m_mainIconLabel->size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    m_mainIconLabel->setPixmap(pixmap);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("infoCardTitle");
    m_titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #e2e8f0;");

    headerLayout->addWidget(m_mainIconLabel);
    headerLayout->addWidget(m_titleLabel, 1);

    auto* metricsGrid = new QGridLayout();
    metricsGrid->setSpacing(10);
    metricsGrid->addWidget(createMetricWidget(AppConfig::ICON_TEMP_SVG, "Temperatura", m_tempValueLabel), 0, 0);
    metricsGrid->addWidget(createMetricWidget(AppConfig::ICON_USAGE_SVG, "Uso", m_usageValueLabel), 0, 1);
    metricsGrid->addWidget(createMetricWidget(AppConfig::ICON_POWER_SVG, "Potência", m_powerValueLabel), 1, 0);
    metricsGrid->addWidget(createMetricWidget(AppConfig::ICON_CLOCK_SVG, "Clock", m_clockValueLabel), 1, 1);

    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(metricsGrid);
}

QWidget* HardwareSummaryCard::createMetricWidget(const QString &iconSvg, const QString &name, QLabel* &valueLabel)
{
    auto* widget = new QWidget();
    auto* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(8);

    auto* iconLabel = new QLabel();
    iconLabel->setFixedSize(16, 16);
    QSvgRenderer renderer;
    renderer.load(iconSvg.toUtf8());
    QPixmap pixmap(iconLabel->size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    iconLabel->setPixmap(pixmap);

    auto* nameLabel = new QLabel(name, this);
    nameLabel->setStyleSheet("font-size: 13px; color: #94a3b8;");

    valueLabel = new QLabel("--", this);
    valueLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #FFFFFF;");
    valueLabel->setAlignment(Qt::AlignRight);

    layout->addWidget(iconLabel);
    layout->addWidget(nameLabel);
    layout->addStretch();
    layout->addWidget(valueLabel);

    return widget;
}

void HardwareSummaryCard::updateMetrics(const HardwareInfo &info)
{
    m_titleLabel->setText(info.name);

    if (info.temperature >= 0) {
        m_tempValueLabel->setText(QString::number(info.temperature, 'f', 1) + " °C");
    } else {
        m_tempValueLabel->setText("--");
    }

    if (info.usage >= 0) {
        m_usageValueLabel->setText(QString::number(info.usage, 'f', 0) + " %");
    } else {
        m_usageValueLabel->setText("--");
    }

    if (info.power >= 0) {
        m_powerValueLabel->setText(QString::number(info.power, 'f', 0) + " W");
    } else {
        m_powerValueLabel->setText("--");
    }

    if (info.clock >= 0) {
        m_clockValueLabel->setText(QString::number(info.clock, 'f', 0) + " MHz");
    } else {
        m_clockValueLabel->setText("--");
    }
}
