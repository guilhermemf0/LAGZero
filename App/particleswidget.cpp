#include "particleswidget.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QResizeEvent>

// Função auxiliar para gerar números decimais aleatórios em um intervalo
static inline qreal randomReal(qreal min, qreal max) {
    return min + (QRandomGenerator::global()->generateDouble() * (max - min));
}

ParticlesWidget::ParticlesWidget(QWidget *parent) : QWidget(parent),
    m_animationTimer(new QTimer(this)),
    m_particleCount(500),
    m_particleSize(1.0),
    // CORREÇÃO: Aumentada a opacidade base das partículas de 150 para 220
    m_particleColor(0, 133, 255, 220)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setMouseTracking(false);

    connect(m_animationTimer, &QTimer::timeout, this, &ParticlesWidget::updateParticles);
}

ParticlesWidget::~ParticlesWidget()
{
    stopAnimation();
}

void ParticlesWidget::initializeParticles()
{
    m_particles.clear();
    for (int i = 0; i < m_particleCount; ++i) {
        Particle p;
        p.position = QPointF(QRandomGenerator::global()->bounded(width()),
                             QRandomGenerator::global()->bounded(height()));
        p.velocity = QPointF(randomReal(-0.5, 0.5),
                             randomReal(-1.5, -0.5));
        // CORREÇÃO: Aumentado o intervalo de opacidade para as partículas ficarem mais fortes
        p.opacity = randomReal(0.4, 1.0);
        m_particles.append(p);
    }
}

void ParticlesWidget::startAnimation()
{
    if (!m_animationTimer->isActive()) {
        if (m_particles.isEmpty()) {
            initializeParticles();
        }
        m_animationTimer->start(30);
    }
}

void ParticlesWidget::stopAnimation()
{
    if (m_animationTimer->isActive()) {
        m_animationTimer->stop();
    }
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
        painter.drawEllipse(p.position, m_particleSize, m_particleSize);
    }
}

void ParticlesWidget::resizeEvent(QResizeEvent *event)
{
    initializeParticles();
    QWidget::resizeEvent(event);
}

void ParticlesWidget::updateParticles()
{
    for (Particle &p : m_particles) {
        p.position += p.velocity;

        if (p.position.y() < -m_particleSize) {
            p.position.setY(height() + m_particleSize);
            p.position.setX(QRandomGenerator::global()->bounded(width()));
            // CORREÇÃO: Aumentado o intervalo de opacidade aqui também
            p.opacity = randomReal(0.4, 1.0);
        }

        if (p.position.x() < -m_particleSize) {
            p.position.setX(width() + m_particleSize);
        } else if (p.position.x() > width() + m_particleSize) {
            p.position.setX(-m_particleSize);
        }
    }
    update();
}
