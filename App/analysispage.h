#ifndef ANALYSISPAGE_H
#define ANALYSISPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QProcess>
#include "staticchartwidget.h"

class AnalysisPage : public QWidget
{
    Q_OBJECT
public:
    explicit AnalysisPage(QWidget *parent = nullptr);
    void refreshSessions(); // Chama isso quando entrar na aba

private slots:
    void onSessionSelected(QListWidgetItem* item);
    void onPythonFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QListWidget* m_sessionList;
    QWidget* m_detailsContainer;
    QVBoxLayout* m_detailsLayout;
    QLabel* m_placeholderLabel;
    StaticChartWidget* m_chart;

    QProcess* m_pythonProcess;
    QString m_currentCsvPath;
    QString m_currentGameName;

    void setupUi();
    void displayResults(const QJsonObject& result);
    QWidget* createInfoCard(const QString& title, const QString& value, const QString& desc, const QString& color);
};

#endif // ANALYSISPAGE_H
