#ifndef STROKEDLABEL_H
#define STROKEDLABEL_H

#include <QWidget>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QFont>

// Classe para renderizar um texto com contorno customizável
class StrokedLabel : public QWidget
{
    Q_OBJECT

public:
    explicit StrokedLabel(const QString &text, QWidget *parent = nullptr);

    // Define as propriedades do texto e do contorno
    void setFont(const QFont &font);
    void setFillBrush(const QBrush &brush);
    void setStrokePen(const QPen &pen);
    void setAlignment(Qt::Alignment alignment);

    // --- ADICIONADO ---
    // Funções para que o layout saiba o tamanho do widget
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    // Sobrescreve o evento de pintura para desenhar o texto customizado
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_text;
    QFont m_font;
    QBrush m_fillBrush;
    QPen m_strokePen;
    Qt::Alignment m_alignment;
};

#endif // STROKEDLABEL_H
