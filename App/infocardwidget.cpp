#include "infocardwidget.h"
#include <QHBoxLayout>
#include <QtSvg/QSvgRenderer>
#include <QPainter>

InfoCardWidget::InfoCardWidget(const QString &iconSvg, const QString &title, QWidget *parent)
    : QWidget{parent}
{
    this->setObjectName("infoCard");
    this->setMinimumHeight(80);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 15, 20, 15);
    layout->setSpacing(15);

    // Ícone
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(32, 32);
    QSvgRenderer renderer;
    renderer.load(iconSvg.toUtf8());
    QPixmap pixmap(m_iconLabel->size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    m_iconLabel->setPixmap(pixmap);

    // Container para Título e Valor
    auto* textContainer = new QWidget(this);
    auto* textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(0,0,0,0);
    textLayout->setSpacing(2);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("infoCardTitle");

    m_valueLabel = new QLabel("N/D", this);
    m_valueLabel->setObjectName("infoCardValue");

    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_valueLabel);

    layout->addWidget(m_iconLabel);
    layout->addWidget(textContainer, 1);
}

void InfoCardWidget::setValue(const QString &value)
{
    m_valueLabel->setText(value);
}

void InfoCardWidget::setValueStyleSheet(const QString &styleSheet)
{
    m_valueLabel->setStyleSheet(styleSheet);
}

void InfoCardWidget::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}
