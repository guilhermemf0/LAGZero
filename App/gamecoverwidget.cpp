#include "gamecoverwidget.h"
#include <QPainter>
#include <QMenu>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPainterPath>
#include <QWidgetAction>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

// As funções auxiliares permanecem as mesmas
QPixmap GameCoverWidget::createRoundedPixmap(const QPixmap& source, int radius)
{
    if (source.isNull()) return {};
    QPixmap result(source.size());
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(result.rect(), radius, radius);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, source);
    return result;
}

QColor GameCoverWidget::extractVibrantColor(const QPixmap &pixmap) {
    if (pixmap.isNull()) return QColor("#66c0f4");
    QImage image = pixmap.toImage().scaled(20, 30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QList<QColor> vibrantColors;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color(image.pixel(x, y));
            if (color.saturationF() > 0.3 && color.valueF() > 0.25 && color.valueF() < 0.95) {
                vibrantColors.append(color);
            }
        }
    }
    if (vibrantColors.isEmpty()) return image.scaled(1, 1).pixelColor(0, 0);
    float r = 0, g = 0, b = 0;
    for (const QColor &color : vibrantColors) {
        r += color.redF(); g += color.greenF(); b += color.blueF();
    }
    return QColor::fromRgbF(r / vibrantColors.size(), g / vibrantColors.size(), b / vibrantColors.size());
}

GameCoverWidget::GameCoverWidget(const QString& gameName, const QString& executableName, const QPixmap& cover, QWidget *parent)
    : QWidget{parent},
    m_executableName(executableName),
    m_originalCover(cover)
{
    // DEPOIS: Voltamos para o tamanho menor que você pediu
    setFixedSize(240, 330);
    setCursor(Qt::PointingHandCursor);
    setToolTip(gameName);

    m_coverLabel = new QLabel(this);
    m_coverLabel->setObjectName("gameCoverForeground");
    m_coverLabel->setScaledContents(true);
    // DEPOIS: Re-centralizamos a capa no novo espaço
    m_coverLabel->setGeometry(45, 52, 150, 225);

    int borderRadius = 10;
    if (!m_originalCover.isNull()) {
        m_coverLabel->setPixmap(createRoundedPixmap(m_originalCover, borderRadius));
    } else {
        m_coverLabel->setStyleSheet(QString("background-color: #1e2b3b; border-radius: %1px;").arg(borderRadius));
    }

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(20);
    m_shadowEffect->setColor(QColor(0, 0, 0, 0)); // Começa transparente
    m_shadowEffect->setOffset(0, 0); // Começa centralizado
    m_coverLabel->setGraphicsEffect(m_shadowEffect);

    m_animationGroup = new QParallelAnimationGroup(this);
    m_dominantColor = extractVibrantColor(m_originalCover);
}

GameCoverWidget::~GameCoverWidget() {}

void GameCoverWidget::enterEvent(QEnterEvent *event)
{
    m_animationGroup->stop();
    m_animationGroup->clear();
    m_coverLabel->raise();

    auto *scaleAnim = new QPropertyAnimation(m_coverLabel, "geometry");
    scaleAnim->setStartValue(m_coverLabel->geometry());
    // DEPOIS: Re-centralizamos a capa expandida
    scaleAnim->setEndValue(QRect(30, 30, 180, 270));
    scaleAnim->setDuration(250);
    scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(scaleAnim);

    // Animação do BLUR
    auto *shadowBlurAnim = new QPropertyAnimation(m_shadowEffect, "blurRadius");
    shadowBlurAnim->setStartValue(m_shadowEffect->blurRadius());
    shadowBlurAnim->setEndValue(45); // DEPOIS: Um blur alto para ser disperso e redondo
    shadowBlurAnim->setDuration(250);
    shadowBlurAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(shadowBlurAnim);

    // Animação do OFFSET
    auto *shadowOffsetAnim = new QPropertyAnimation(m_shadowEffect, "offset");
    shadowOffsetAnim->setStartValue(m_shadowEffect->offset());
    shadowOffsetAnim->setEndValue(QPointF(0, 0)); // Mantém centralizado para o efeito redondo
    shadowOffsetAnim->setDuration(250);
    shadowOffsetAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(shadowOffsetAnim);

    // Animação da COR
    auto *shadowColorAnim = new QPropertyAnimation(m_shadowEffect, "color");
    shadowColorAnim->setStartValue(m_shadowEffect->color());
    QColor glowColor = m_dominantColor;
    glowColor.setAlpha(180); // Opacidade forte mas não total para um brilho "fraco"
    shadowColorAnim->setEndValue(glowColor);
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

    auto *scaleAnim = new QPropertyAnimation(m_coverLabel, "geometry");
    scaleAnim->setStartValue(m_coverLabel->geometry());
    // DEPOIS: Voltamos para a nova posição de descanso
    scaleAnim->setEndValue(QRect(45, 52, 150, 225));
    scaleAnim->setDuration(250);
    scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(scaleAnim);

    // Animações de retorno do brilho
    auto *shadowBlurAnim = new QPropertyAnimation(m_shadowEffect, "blurRadius");
    shadowBlurAnim->setStartValue(m_shadowEffect->blurRadius());
    shadowBlurAnim->setEndValue(20);
    shadowBlurAnim->setDuration(250);
    shadowBlurAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(shadowBlurAnim);

    auto *shadowOffsetAnim = new QPropertyAnimation(m_shadowEffect, "offset");
    shadowOffsetAnim->setStartValue(m_shadowEffect->offset());
    shadowOffsetAnim->setEndValue(QPointF(0, 0)); // Mantém centralizado
    shadowOffsetAnim->setDuration(250);
    shadowOffsetAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_animationGroup->addAnimation(shadowOffsetAnim);

    auto *shadowColorAnim = new QPropertyAnimation(m_shadowEffect, "color");
    shadowColorAnim->setStartValue(m_shadowEffect->color());
    shadowColorAnim->setEndValue(QColor(0, 0, 0, 0)); // Volta a ser transparente
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

