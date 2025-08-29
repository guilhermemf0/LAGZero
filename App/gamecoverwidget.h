#ifndef GAMECOVERWIDGET_H
#define GAMECOVERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QContextMenuEvent> // NOVO

class GameCoverWidget : public QWidget
{
    Q_OBJECT
public:
    // ALTERADO: Adicionado executableName ao construtor
    explicit GameCoverWidget(const QString& gameName, const QString& executableName, const QPixmap& cover, QWidget *parent = nullptr);
    ~GameCoverWidget();

signals:
    // NOVO: Sinais para o menu de contexto
    void editGameRequested(const QString& executableName);
    void removeGameRequested(const QString& executableName);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    // NOVO: Sobrescreve o evento de menu de contexto
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QLabel *m_coverLabel;
    QLabel *m_nameLabel;
    QWidget *m_container;
    QPropertyAnimation *m_animation;
    // NOVO: Armazena o nome do executável para o menu de contexto
    QString m_executableName;
};

#endif // GAMECOVERWIDGET_H
