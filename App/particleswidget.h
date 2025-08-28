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
    qreal size; // Adicionado tamanho individual
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
    // Adicionado para interatividade com o mouse
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void updateParticles();

private:
    QTimer *m_animationTimer;
    QVector<Particle> m_particles;
    int m_particleCount;
    QColor m_particleColor;

    // Variáveis para interatividade
    QPointF m_mousePos;
    bool m_mouseInside = false;

    void initializeParticles();
};

#endif // PARTICLESWIDGET_H
