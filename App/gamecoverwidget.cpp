#include "gamecoverwidget.h"
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QMenu>

GameCoverWidget::GameCoverWidget(const QString& gameName, const QString& executableName, const QPixmap& cover, QWidget *parent)
    : QWidget{parent},
    m_executableName(executableName)
{
    setFixedSize(150, 225);
    setCursor(Qt::PointingHandCursor);

    m_container = new QWidget(this);
    m_container->setFixedSize(130, 195);
    m_container->move(10, 15);

    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 120));
    shadow->setOffset(0, 5);
    m_container->setGraphicsEffect(shadow);

    m_coverLabel = new QLabel(m_container);
    m_coverLabel->setGeometry(m_container->rect());
    m_coverLabel->setScaledContents(true);
    if (!cover.isNull()) {
        m_coverLabel->setPixmap(cover);
    }
    m_coverLabel->setStyleSheet("background-color: #111; border-radius: 8px; border: 1px solid rgba(255, 255, 255, 0.1);");

    m_nameLabel = new QLabel(gameName, m_coverLabel);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 0.75);
        color: white;
        font-size: 14px;
        font-weight: bold;
        padding: 5px;
        border-bottom-left-radius: 8px;
        border-bottom-right-radius: 8px;
        border-top: 1px solid rgba(255, 255, 255, 0.1);
    )");
    m_nameLabel->hide();

    m_animation = new QPropertyAnimation(m_container, "pos", this);
    m_animation->setDuration(150);
    m_animation->setEasingCurve(QEasingCurve::OutQuad);
}

GameCoverWidget::~GameCoverWidget() {}

// NOVO: Implementação do menu de contexto
void GameCoverWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu contextMenu(this);
    QAction *editAction = contextMenu.addAction("Corrigir Identificação");
    QAction *changeCoverAction = contextMenu.addAction("Trocar Capa");
    contextMenu.addSeparator();
    QAction *removeAction = contextMenu.addAction("Remover Jogo");

    QAction *selectedAction = contextMenu.exec(event->globalPos());

    if (selectedAction == editAction) {
        emit editGameRequested(m_executableName);
    } else if (selectedAction == changeCoverAction) {
        emit changeCoverRequested(m_executableName);
    } else if (selectedAction == removeAction) {
        emit removeGameRequested(m_executableName);
    }
}

void GameCoverWidget::enterEvent(QEnterEvent *event)
{
    m_nameLabel->setGeometry(0, m_coverLabel->height() - m_nameLabel->sizeHint().height(), m_coverLabel->width(), m_nameLabel->sizeHint().height());
    m_nameLabel->show();

    m_animation->setStartValue(m_container->pos());
    m_animation->setEndValue(QPoint(10, 5));
    m_animation->start();

    QWidget::enterEvent(event);
}

void GameCoverWidget::leaveEvent(QEvent *event)
{
    m_nameLabel->hide();

    m_animation->setStartValue(m_container->pos());
    m_animation->setEndValue(QPoint(10, 15));
    m_animation->start();

    QWidget::leaveEvent(event);
}
