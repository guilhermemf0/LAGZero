#ifndef COVERSELECTIONDIALOG_H
#define COVERSELECTIONDIALOG_H

#include <QDialog>
#include <QList>
#include <QJsonObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QGridLayout>
#include <QScrollArea>

class CoverSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CoverSelectionDialog(const QList<QJsonObject> &covers, QWidget *parent = nullptr);
    QString getSelectedUrl() const;

private:
    void onCoverClicked(const QString& url);

    QList<QJsonObject> m_covers;
    QString m_selectedUrl;
    QNetworkAccessManager *m_netManager;
    QGridLayout *m_gridLayout;
    QScrollArea *m_scrollArea;
};

#endif // COVERSELECTIONDIALOG_H
