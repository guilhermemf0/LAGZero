#include "infocardwidget.h"
#include <QHBoxLayout>
#include <QtSvg/QSvgRenderer>
#include <QPainter>
#include <QStyle> // Adicionado para polish

InfoCardWidget::InfoCardWidget(const QString &iconSvg, const QString &title, QWidget *parent)
    : QWidget{parent}
{
    // 1. Define a CLASSE do widget pai para ".InfoCard"
    this->setProperty("class", "InfoCard");
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
    // 2. Define a CLASSE do título para ".Title"
    m_titleLabel->setProperty("class", "Title");

    m_valueLabel = new QLabel("N/D", this);
    // 3. Define a CLASSE do valor para ".Value"
    m_valueLabel->setProperty("class", "Value");

    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_valueLabel);

    layout->addWidget(m_iconLabel);
    layout->addWidget(textContainer, 1);

    // 4. Força o widget a reler o stylesheet
    // (Necessário para widgets criados dinamicamente)
    this->style()->unpolish(this);
    this->style()->polish(this);
    m_titleLabel->style()->unpolish(m_titleLabel);
    m_titleLabel->style()->polish(m_titleLabel);
    m_valueLabel->style()->unpolish(m_valueLabel);
    m_valueLabel->style()->polish(m_valueLabel);
}

void InfoCardWidget::setValue(const QString &value)
{
    m_valueLabel->setText(value);
}

void InfoCardWidget::setValueStyleSheet(const QString &styleSheet)
{
    // Adiciona o ; para corrigir o bug de parse
    m_valueLabel->setStyleSheet(styleSheet + ";");
}

void InfoCardWidget::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}
