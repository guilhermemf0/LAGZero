#ifndef INFOCARDWIDGET_H
#define INFOCARDWIDGET_H

#include <QWidget>
#include <QLabel>

// Novo widget para exibir um card de informação com ícone, título e valor.
// Usado nas telas de temperatura para um visual mais consistente.
class InfoCardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit InfoCardWidget(const QString &iconSvg, const QString &title, QWidget *parent = nullptr);
    void setValue(const QString &value);
    void setValueStyleSheet(const QString &styleSheet);
    void setTitle(const QString &title);

private:
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QLabel *m_valueLabel;
};

#endif // INFOCARDWIDGET_H
