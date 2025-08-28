#include "particleswidget.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <cmath>

static inline qreal randomReal(qreal min, qreal max) {
    return min + (QRandomGenerator::global()->generateDouble() * (max - min));
}

ParticlesWidget::ParticlesWidget(QWidget *parent) : QWidget(parent),
    m_animationTimer(new QTimer(this)),
    m_particleCount(200), // Reduzido para um visual mais limpo
    m_particleColor(0, 133, 255)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, true); // Deixa o mouse passar
    setMouseTracking(true);

    connect(m_animationTimer, &QTimer::timeout, this, &ParticlesWidget::updateParticles);
}

ParticlesWidget::~ParticlesWidget() { stopAnimation(); }

void ParticlesWidget::initializeParticles()
{
    m_particles.clear();
    for (int i = 0; i < m_particleCount; ++i) {
        Particle p;
        p.position = QPointF(QRandomGenerator::global()->bounded(width()), QRandomGenerator::global()->bounded(height()));
        p.size = randomReal(1.0, 3.0);
        // Efeito de paralaxe: partículas maiores são mais lentas e "próximas"
        p.velocity = QPointF(randomReal(-0.2, 0.2), randomReal(-0.5, -0.1)) * (p.size / 3.0);
        p.opacity = randomReal(0.1, 0.6);
        m_particles.append(p);
    }
}

void ParticlesWidget::startAnimation()
{
    if (!m_animationTimer->isActive()) {
        if (m_particles.isEmpty()) initializeParticles();
        m_animationTimer->start(16); // ~60 FPS
    }
}

void ParticlesWidget::stopAnimation()
{
    if (m_animationTimer->isActive()) m_animationTimer->stop();
}

void ParticlesWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const Particle &p : m_particles) {
        QColor particleColor = m_particleColor;
        particleColor.setAlphaF(p.opacity);
        painter.setBrush(particleColor);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(p.position, p.size, p.size);
    }
}

void ParticlesWidget::resizeEvent(QResizeEvent *event)
{
    initializeParticles();
    QWidget::resizeEvent(event);
}

void ParticlesWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePos = event->pos();
    m_mouseInside = true;
    // Não chama o evento base para não interferir com widgets abaixo
}

void ParticlesWidget::leaveEvent(QEvent *event)
{
    m_mouseInside = false;
    QWidget::leaveEvent(event);
}

void ParticlesWidget::updateParticles()
{
    for (Particle &p : m_particles) {
        if (m_mouseInside) {
            qreal dx = p.position.x() - m_mousePos.x();
            qreal dy = p.position.y() - m_mousePos.y();
            qreal distance = std::sqrt(dx*dx + dy*dy);
            qreal maxDist = 120.0;

            if (distance < maxDist) {
                qreal force = (maxDist - distance) / maxDist;
                p.position.rx() += (dx / distance) * force * 2.0;
                p.position.ry() += (dy / distance) * force * 2.0;
            }
        }

        p.position += p.velocity;

        if (p.position.y() < -p.size) p.position.setY(height() + p.size);
        if (p.position.x() < -p.size) p.position.setX(width() + p.size);
        else if (p.position.x() > width() + p.size) p.position.setX(-p.size);
    }
    update();
}
