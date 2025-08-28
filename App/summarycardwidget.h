#ifndef SUMMARYCARDWIDGET_H
#define SUMMARYCARDWIDGET_H

#include <QWidget>
#include <QLabel>

// Novo widget para os cards de resumo na tela de Visão Geral.
// Possui um layout diferente, com o valor em destaque acima do título.
class SummaryCardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SummaryCardWidget(const QString &title, QWidget *parent = nullptr);
    void setValue(const QString &value);
    void setColor(const QString &color);

private:
    QLabel *m_titleLabel;
    QLabel *m_valueLabel;
};

#endif // SUMMARYCARDWIDGET_H
