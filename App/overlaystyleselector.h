#ifndef OVERLAYSTYLESELECTOR_H
#define OVERLAYSTYLESELECTOR_H

#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QPainter>
#include <QFontMetrics>

class OverlayStylePreview : public QPushButton
{
    Q_OBJECT
public:
    explicit OverlayStylePreview(int styleId, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_styleId;
    void drawStyleDetailed(QPainter &painter);
    void drawStyleCompact(QPainter &painter);
    // ADICIONADO: A declaração que estava faltando
    void drawMetric(QPainter &painter, QFontMetrics &fm, int &y, const QColor& color, const QString& label, const QString& value, const QString& unit);
};

class OverlayStyleSelector : public QWidget
{
    Q_OBJECT
public:
    explicit OverlayStyleSelector(QWidget *parent = nullptr);
    QButtonGroup* buttonGroup() const { return m_buttonGroup; }

private:
    QButtonGroup *m_buttonGroup;
};

#endif // OVERLAYSTYLESELECTOR_H
