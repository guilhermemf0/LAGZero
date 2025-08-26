#ifndef GAMECOVERWIDGET_H
#define GAMECOVERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>

class GameCoverWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GameCoverWidget(const QString& gameName, const QPixmap& cover, QWidget *parent = nullptr);
    ~GameCoverWidget();

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QLabel *m_coverLabel;
    QLabel *m_nameLabel;
    QWidget *m_container; // Contêiner para a animação
    QPropertyAnimation *m_animation;
};

#endif // GAMECOVERWIDGET_H
