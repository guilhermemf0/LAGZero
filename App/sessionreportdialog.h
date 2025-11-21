#ifndef SESSIONREPORTDIALOG_H
#define SESSIONREPORTDIALOG_H

#include <QDialog>
#include <QJsonObject>

class QVBoxLayout;
class QLabel;

class SessionReportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SessionReportDialog(const QJsonObject& data, const QString& gameName, QWidget *parent = nullptr);

private:
    void setupUi();
    QWidget* createCalculusCard(const QString& title, const QString& value, const QString& description, const QString& iconPath);

    QJsonObject m_data;
    QString m_gameName;
};

#endif // SESSIONREPORTDIALOG_H
