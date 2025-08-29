#include "coverselectiondialog.h"
#include "clickablelabel.h"
#include <QNetworkReply>
#include <QPixmap>
#include <QVBoxLayout>
#include <QBuffer>
#include <QDebug>

CoverSelectionDialog::CoverSelectionDialog(const QList<QJsonObject> &covers, QWidget *parent)
    : QDialog(parent), m_covers(covers)
{
    this->setWindowTitle("Selecione uma Capa");
    this->setMinimumSize(650, 500);

    m_netManager = new QNetworkAccessManager(this);

    auto* mainLayout = new QVBoxLayout(this);
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    auto* scrollWidget = new QWidget();
    m_gridLayout = new QGridLayout(scrollWidget);
    m_scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(m_scrollArea);

    int row = 0, col = 0;
    const int maxCols = 4;

    for (const QJsonObject& cover : m_covers) {
        QString thumbnailUrl = cover["url"].toString();

        auto* coverLabel = new ClickableLabel();
        coverLabel->setFixedSize(150, 225);
        coverLabel->setStyleSheet("background-color: #1e293b; border-radius: 8px;");
        coverLabel->setCursor(Qt::PointingHandCursor);
        coverLabel->setAlignment(Qt::AlignCenter);
        coverLabel->setText("Carregando...");

        connect(coverLabel, &ClickableLabel::clicked, this, [this, thumbnailUrl](){
            onCoverClicked(thumbnailUrl);
        });

        m_gridLayout->addWidget(coverLabel, row, col);

        // --- CORREÇÃO APLICADA AQUI ---
        // Usamos chaves {} para criar a QUrl, evitando a ambiguidade.
        QNetworkRequest request{QUrl{thumbnailUrl}};
        QNetworkReply* reply = m_netManager->get(request);
        // --------------------------------

        connect(reply, &QNetworkReply::finished, this, [coverLabel, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QPixmap pixmap;
                pixmap.loadFromData(reply->readAll());
                coverLabel->setPixmap(pixmap.scaled(coverLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                coverLabel->setText("");
            } else {
                qWarning() << "Falha ao baixar imagem de capa:" << reply->errorString();
                coverLabel->setText("Erro");
            }
            reply->deleteLater();
        });

        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }

    if (m_covers.isEmpty()) {
        auto* noCoversLabel = new QLabel("Nenhuma capa em pé encontrada para este jogo.");
        noCoversLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(noCoversLabel);
    }
}

QString CoverSelectionDialog::getSelectedUrl() const
{
    return m_selectedUrl;
}

void CoverSelectionDialog::onCoverClicked(const QString &url)
{
    m_selectedUrl = url;
    accept(); // Fecha o diálogo e retorna QDialog::Accepted
}
