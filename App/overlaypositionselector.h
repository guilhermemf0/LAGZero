#ifndef OVERLAYPOSITIONSELECTOR_H
#define OVERLAYPOSITIONSELECTOR_H

#include <QWidget>
#include <QButtonGroup>

class OverlayPositionSelector : public QWidget
{
    Q_OBJECT
public:
    explicit OverlayPositionSelector(QWidget *parent = nullptr);
    void setCurrentPosition(int id);

signals:
    void positionSelected(int id);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateRects();

    int m_currentPosition = 0;
    QRectF m_rects[4];
};

#endif // OVERLAYPOSITIONSELECTOR_H
