#ifndef GAMECOVERWIDGET_H
#define GAMECOVERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QContextMenuEvent>
#include <QGraphicsEffect>
#include <QParallelAnimationGroup>
#include <QFrame>
#include <QVariantAnimation>

class QGraphicsOpacityEffect;
class QGraphicsDropShadowEffect;
class QVariantAnimation;

class GameCoverWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GameCoverWidget(const QString& gameName, const QString& executableName, const QPixmap& cover, QWidget *parent = nullptr);
    ~GameCoverWidget();

signals:
    void editGameRequested(const QString& executableName);
    void removeGameRequested(const QString& executableName);
    void changeCoverRequested(const QString& executableName);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void createReflection();

    // UI Widgets
    QFrame* m_glowFrame = nullptr;
    QLabel *m_coverLabel;
    QLabel *m_reflectionLabel;

    // Effects
    QGraphicsDropShadowEffect* m_shadowEffect;
    QGraphicsDropShadowEffect* m_glowEffect = nullptr;
    QGraphicsOpacityEffect* m_glowOpacityEffect = nullptr;

    // Animação auxiliar
    QVariantAnimation* m_blurAnimation = nullptr;

    // Animation Control
    QParallelAnimationGroup *m_animationGroup;

    // Data
    QString m_executableName;
    QPixmap m_originalCover;
};

#endif // GAMECOVERWIDGET_H
