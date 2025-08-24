#include "strokedlabel.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QPainterPathStroker> // Incluído para a correção do contorno

StrokedLabel::StrokedLabel(const QString &text, QWidget *parent)
    : QWidget(parent),
      m_text(text),
      m_fillBrush(Qt::white),
      m_strokePen(QPen(Qt::black, 1)),
      m_alignment(Qt::AlignCenter)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void StrokedLabel::setFont(const QFont &font)
{
    m_font = font;
    updateGeometry();
    update();
}

void StrokedLabel::setFillBrush(const QBrush &brush)
{
    m_fillBrush = brush;
    update();
}

void StrokedLabel::setStrokePen(const QPen &pen)
{
    m_strokePen = pen;
    updateGeometry();
    update();
}

void StrokedLabel::setAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;
    update();
}

QSize StrokedLabel::sizeHint() const
{
    QFontMetrics fm(m_font);
    QRect textRect = fm.boundingRect(m_text);
    int margin = m_strokePen.width() * 2;
    return QSize(textRect.width() + margin, textRect.height() + margin);
}

QSize StrokedLabel::minimumSizeHint() const
{
    return sizeHint();
}


void StrokedLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Cria o caminho do texto original
    QPainterPath textPath;
    textPath.addText(0, 0, m_font, m_text);

    // --- CORREÇÃO DEFINITIVA: Usa QPainterPathStroker para criar o contorno ---
    QPainterPathStroker stroker;
    stroker.setWidth(m_strokePen.width());
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);

    QPainterPath strokePath = stroker.createStroke(textPath);

    // Centraliza o texto e o contorno
    QRectF totalBoundingRect = strokePath.boundingRect().united(textPath.boundingRect());
    qreal x = (width() - totalBoundingRect.width()) / 2 - totalBoundingRect.left();
    qreal y = (height() - totalBoundingRect.height()) / 2 - totalBoundingRect.top();
    painter.translate(x, y);

    // 1. Desenha o contorno (agora como uma forma preenchida)
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_strokePen.color());
    painter.drawPath(strokePath);

    // 2. Desenha o preenchimento do texto original por cima
    painter.setBrush(m_fillBrush);
    painter.drawPath(textPath);
}
