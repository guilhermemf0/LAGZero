#include "summarycardwidget.h"
#include <QVBoxLayout>

SummaryCardWidget::SummaryCardWidget(const QString &title, QWidget *parent)
    : QWidget{parent}
{
    this->setObjectName("summaryCard");
    this->setMinimumSize(140, 90);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(5);

    m_valueLabel = new QLabel("---", this);
    m_valueLabel->setObjectName("summaryCardValue");
    m_valueLabel->setAlignment(Qt::AlignLeft);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("summaryCardTitle");
    m_titleLabel->setAlignment(Qt::AlignLeft);

    layout->addWidget(m_valueLabel);
    layout->addWidget(m_titleLabel);
    layout->addStretch();
}

void SummaryCardWidget::setValue(const QString &value)
{
    m_valueLabel->setText(value);
}

void SummaryCardWidget::setColor(const QString &color)
{
    m_valueLabel->setStyleSheet(QString("color: %1;").arg(color));
}
