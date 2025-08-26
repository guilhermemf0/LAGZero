#ifndef PARTICLESWIDGET_H
#define PARTICLESWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QResizeEvent>

struct Particle {
    QPointF position;
    QPointF velocity;
    qreal opacity;
};

class ParticlesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ParticlesWidget(QWidget *parent = nullptr);
    ~ParticlesWidget();

    void startAnimation();
    void stopAnimation();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateParticles();

private:
    QTimer *m_animationTimer;
    QVector<Particle> m_particles;
    int m_particleCount;
    qreal m_particleSize;
    QColor m_particleColor;

    void initializeParticles();
};

#endif // PARTICLESWIDGET_H
