#ifndef GAMECOVERWIDGET_H
#define GAMECOVERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QContextMenuEvent>
#include <QParallelAnimationGroup>
#include <QPixmap>
#include <QColor>
#include <QGraphicsEffect> // Adicionado

class QGraphicsDropShadowEffect; // Forward declaration

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

private:
    // UI Widgets
    QLabel *m_coverLabel;
    QGraphicsDropShadowEffect* m_shadowEffect; // De volta!

    // Animation Control
    QParallelAnimationGroup *m_animationGroup;

    // Data
    QString m_executableName;
    QPixmap m_originalCover;
    QColor m_dominantColor;

    // Funções Auxiliares
    QPixmap createRoundedPixmap(const QPixmap& source, int radius);
    QColor extractVibrantColor(const QPixmap &pixmap);
};

#endif // GAMECOVERWIDGET_H

