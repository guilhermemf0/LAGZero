#ifndef OVERLAYSTYLESELECTOR_H
#define OVERLAYSTYLESELECTOR_H

#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>

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
