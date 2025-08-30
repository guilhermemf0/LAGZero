#include "gamecoverwidget.h"
#include <QPainter>
#include <QMenu>
#include <QGraphicsDropShadowEffect>
#include <QWidgetAction>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QLinearGradient>
#include <QTransform>
#include <QPropertyAnimation>
#include <QDebug>
#include <QVBoxLayout>
#include <QVariantAnimation>

GameCoverWidget::GameCoverWidget(const QString& gameName, const QString& executableName, const QPixmap& cover, QWidget *parent)
    : QWidget{parent},
    m_executableName(executableName),
    m_originalCover(cover)
{
    setFixedSize(180, 320);
    setCursor(Qt::PointingHandCursor);
    setToolTip(gameName);

    // Label da Capa
    m_coverLabel = new QLabel(this);
    m_coverLabel->setObjectName("gameCoverForeground");
    m_coverLabel->setScaledContents(true);
    m_coverLabel->setGeometry(15, 15, 150, 225);
    if (!m_originalCover.isNull()) {
        m_coverLabel->setPixmap(m_originalCover);
    } else {
        m_coverLabel->setStyleSheet("background-color: #1e293b; border-radius: 8px;");
    }

    // Efeito de sombra
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(15);
    m_shadowEffect->setColor(QColor(0, 0, 0, 180));
    m_shadowEffect->setOffset(0, 5);
    m_coverLabel->setGraphicsEffect(m_shadowEffect);

    // O m_reflectionLabel foi removido
    m_reflectionLabel = nullptr;

    // Animação auxiliar
    m_blurAnimation = nullptr;

    m_animationGroup = new QParallelAnimationGroup(this);
}

GameCoverWidget::~GameCoverWidget() {}

void GameCoverWidget::createReflection()
{
    // A lógica de reflexo foi removida
}

void GameCoverWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_coverLabel->setGeometry(15, 15, 150, 225);
    QWidget::resizeEvent(event);
}

void GameCoverWidget::enterEvent(QEnterEvent *event)
{
    m_animationGroup->stop();
    m_animationGroup->clear();
    m_coverLabel->raise();

    // Animação para o zoom da capa
    auto *scaleAnim = new QPropertyAnimation(m_coverLabel, "geometry");
    scaleAnim->setStartValue(m_coverLabel->geometry());
    scaleAnim->setEndValue(QRect(0, 0, 180, 270));
    scaleAnim->setDuration(250);
    scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(scaleAnim);

    // Animação do brilho
    auto *shadowBlurAnim = new QPropertyAnimation(m_shadowEffect, "blurRadius");
    shadowBlurAnim->setStartValue(m_shadowEffect->blurRadius());
    shadowBlurAnim->setEndValue(30);
    shadowBlurAnim->setDuration(250);
    shadowBlurAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(shadowBlurAnim);

    auto *shadowOffsetAnim = new QPropertyAnimation(m_shadowEffect, "offset");
    shadowOffsetAnim->setStartValue(m_shadowEffect->offset());
    shadowOffsetAnim->setEndValue(QPointF(0, 0));
    shadowOffsetAnim->setDuration(250);
    shadowOffsetAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(shadowOffsetAnim);

    auto *shadowColorAnim = new QPropertyAnimation(m_shadowEffect, "color");
    shadowColorAnim->setStartValue(m_shadowEffect->color());
    shadowColorAnim->setEndValue(QColor("#66c0f4"));
    shadowColorAnim->setDuration(250);
    shadowColorAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(shadowColorAnim);

    m_animationGroup->start();

    QWidget::enterEvent(event);
}

void GameCoverWidget::leaveEvent(QEvent *event)
{
    m_animationGroup->stop();
    m_animationGroup->clear();

    // Volta a capa ao seu estado original
    auto *scaleAnim = new QPropertyAnimation(m_coverLabel, "geometry");
    scaleAnim->setStartValue(m_coverLabel->geometry());
    scaleAnim->setEndValue(QRect(15, 15, 150, 225));
    scaleAnim->setDuration(250);
    scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(scaleAnim);

    // Volta a sombra escura ao estado original
    auto *shadowBlurAnim = new QPropertyAnimation(m_shadowEffect, "blurRadius");
    shadowBlurAnim->setStartValue(m_shadowEffect->blurRadius());
    shadowBlurAnim->setEndValue(15);
    shadowBlurAnim->setDuration(250);
    shadowBlurAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(shadowBlurAnim);

    auto *shadowOffsetAnim = new QPropertyAnimation(m_shadowEffect, "offset");
    shadowOffsetAnim->setStartValue(m_shadowEffect->offset());
    shadowOffsetAnim->setEndValue(QPointF(0, 5));
    shadowOffsetAnim->setDuration(250);
    shadowOffsetAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(shadowOffsetAnim);

    auto *shadowColorAnim = new QPropertyAnimation(m_shadowEffect, "color");
    shadowColorAnim->setStartValue(m_shadowEffect->color());
    shadowColorAnim->setEndValue(QColor(0, 0, 0, 180));
    shadowColorAnim->setDuration(250);
    shadowColorAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(shadowColorAnim);

    m_animationGroup->start();

    QWidget::leaveEvent(event);
}

void GameCoverWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu contextMenu(this);
    QWidgetAction* titleAction = new QWidgetAction(&contextMenu);
    QLabel* titleLabel = new QLabel(toolTip());
    titleLabel->setStyleSheet("font-weight: bold; padding: 5px 10px; background-color: #1e293b; color: white;");
    titleAction->setDefaultWidget(titleLabel);
    contextMenu.addAction(titleAction);
    contextMenu.addSeparator();
    QAction *editAction = contextMenu.addAction("Corrigir Identificação");
    QAction *changeCoverAction = contextMenu.addAction("Trocar Capa");
    contextMenu.addSeparator();
    QAction *removeAction = contextMenu.addAction("Remover Jogo");
    QAction *selectedAction = contextMenu.exec(event->globalPos());

    if (selectedAction == editAction) emit editGameRequested(m_executableName);
    else if (selectedAction == changeCoverAction) emit changeCoverRequested(m_executableName);
    else if (selectedAction == removeAction) emit removeGameRequested(m_executableName);
}
