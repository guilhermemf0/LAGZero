#ifndef GLASSWIDGET_H
#define GLASSWIDGET_H

#include <QWidget>
#include <QPoint>

class GlassWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GlassWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    // CORREÇÃO: Adicionada a declaração da função em falta
    void enterEvent(QEnterEvent *event) override;

private:
    QPoint m_mousePos;
    bool m_mouseInside = false;
};

#endif // GLASSWIDGET_H
