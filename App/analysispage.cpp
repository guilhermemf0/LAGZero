#include "analysispage.h"
#include "databasemanager.h"
#include "appconstants.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDebug>
#include <QtSvg/QSvgRenderer>
#include <QPainter>
#include <QPushButton>
#include <QMessageBox>

AnalysisPage::AnalysisPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
    m_pythonProcess = new QProcess(this);
    connect(m_pythonProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &AnalysisPage::onPythonFinished);
}

void AnalysisPage::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* leftWidget = new QWidget();
    leftWidget->setFixedWidth(300);
    leftWidget->setStyleSheet("background-color: #0d1117; border-right: 1px solid #30363d;");
    auto* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(10, 10, 10, 10);
    leftLayout->setSpacing(10);

    auto* listTitle = new QLabel("Histórico de Sessões");
    listTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #8b949e; border: none;");
    leftLayout->addWidget(listTitle);

    m_sessionList = new QListWidget();
    m_sessionList->setStyleSheet("QListWidget { background: transparent; border: none; color: #c9d1d9; font-size: 13px; outline: 0; }"
                                 "QListWidget::item { padding: 12px; border-bottom: 1px solid #21262d; border-radius: 6px; }"
                                 "QListWidget::item:selected { background: #1f6feb; color: white; }"
                                 "QListWidget::item:hover:!selected { background: #161b22; }");
    connect(m_sessionList, &QListWidget::itemClicked, this, &AnalysisPage::onSessionSelected);
    leftLayout->addWidget(m_sessionList);

    auto* deleteBtn = new QPushButton("Deletar Sessão Selecionada");
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setStyleSheet("QPushButton { background-color: #da3633; color: white; border-radius: 6px; padding: 8px; font-weight: bold; border: none; }"
                             "QPushButton:hover { background-color: #b62324; }");
    connect(deleteBtn, &QPushButton::clicked, this, [this](){
        QListWidgetItem* item = m_sessionList->currentItem();
        if (!item) return;

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirmar Exclusão", "Tem certeza que deseja apagar esta análise e seus dados?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            int id = item->data(Qt::UserRole + 2).toInt();
            if (DatabaseManager::instance().deleteSession(id)) {
                refreshSessions();
                QLayoutItem* child;
                while ((child = m_detailsLayout->takeAt(0)) != 0) {
                    if (child->widget()) child->widget()->deleteLater();
                    delete child;
                }
                m_placeholderLabel = new QLabel("Selecione uma sessão para ver a análise.");
                m_placeholderLabel->setStyleSheet("color: #8b949e; font-size: 16px; border: none;");
                m_placeholderLabel->setAlignment(Qt::AlignCenter);
                m_detailsLayout->addWidget(m_placeholderLabel);
            }
        }
    });
    leftLayout->addWidget(deleteBtn);
    mainLayout->addWidget(leftWidget);

    auto* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; } QScrollBar:vertical { background: #0d1117; width: 12px; }");

    m_detailsContainer = new QWidget();
    m_detailsContainer->setStyleSheet("background: transparent;");
    m_detailsLayout = new QVBoxLayout(m_detailsContainer);
    m_detailsLayout->setSpacing(20);
    m_detailsLayout->setAlignment(Qt::AlignTop);
    m_detailsLayout->setContentsMargins(30, 30, 30, 30);

    m_placeholderLabel = new QLabel("Selecione uma sessão ao lado para iniciar a análise.");
    m_placeholderLabel->setStyleSheet("color: #8b949e; font-size: 16px; border: none;");
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_detailsLayout->addWidget(m_placeholderLabel);

    scrollArea->setWidget(m_detailsContainer);
    mainLayout->addWidget(scrollArea, 1);
}

void AnalysisPage::refreshSessions()
{
    m_sessionList->clear();
    auto sessions = DatabaseManager::instance().getAllSessions();
    for (const auto& s : sessions) {
        QString itemText = QString("%1\n%2 • %3").arg(s.gameName, s.startTime, s.duration);
        auto* item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, s.reportPath);
        item->setData(Qt::UserRole + 1, s.gameName);
        item->setData(Qt::UserRole + 2, s.id);
        m_sessionList->addItem(item);
    }
}

void AnalysisPage::onSessionSelected(QListWidgetItem* item)
{
    QString csvPath = item->data(Qt::UserRole).toString();
    m_currentGameName = item->data(Qt::UserRole + 1).toString();
    m_currentCsvPath = csvPath;

    QLayoutItem* child;
    while ((child = m_detailsLayout->takeAt(0)) != 0) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    auto* loading = new QLabel("Executando Análise Matemática Avançada...");
    loading->setStyleSheet("color: #58a6ff; font-weight: bold; font-size: 16px; margin-top: 50px; border: none;");
    loading->setAlignment(Qt::AlignCenter);
    m_detailsLayout->addWidget(loading);

    QString scriptPath = QCoreApplication::applicationDirPath() + "/analise_sessao.py";
    m_pythonProcess->start("python", QStringList() << scriptPath << csvPath);
}

void AnalysisPage::onPythonFinished(int, QProcess::ExitStatus)
{
    QLayoutItem* child;
    while ((child = m_detailsLayout->takeAt(0)) != 0) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    QByteArray output = m_pythonProcess->readAllStandardOutput();
    QJsonDocument doc = QJsonDocument::fromJson(output);

    if (doc.isNull() || !doc["sucesso"].toBool()) {
        auto* err = new QLabel("Erro na análise matemática.\nVerifique se o arquivo CSV ainda existe ou se contém dados suficientes.");
        err->setStyleSheet("color: #f85149; font-size: 14px; border: none;");
        err->setAlignment(Qt::AlignCenter);
        m_detailsLayout->addWidget(err);
        return;
    }

    displayResults(doc.object());
}

QWidget* createStatCard(const QString& label, const QString& value, const QString& color) {
    auto* w = new QWidget();
    auto* l = new QVBoxLayout(w);
    l->setContentsMargins(0,0,0,0); l->setSpacing(2);
    auto* lbl = new QLabel(label); lbl->setStyleSheet("color: #8b949e; font-size: 12px; border: none;");
    auto* val = new QLabel(value); val->setStyleSheet(QString("color: %1; font-size: 18px; font-weight: bold; border: none;").arg(color));
    l->addWidget(lbl); l->addWidget(val);
    return w;
}

QWidget* createMathCard(const QString& title, const QString& value, const QString& description, const QString& color) {
    auto* card = new QFrame();
    card->setStyleSheet(QString("QFrame { background: #161b22; border-radius: 8px; border-left: 4px solid %1; }").arg(color));
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(15, 15, 15, 15);

    auto* t = new QLabel(title); t->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 15px; border: none;").arg(color));
    auto* v = new QLabel(value); v->setStyleSheet("color: white; font-size: 22px; font-weight: bold; margin: 5px 0; border: none;");
    auto* d = new QLabel(description); d->setStyleSheet("color: #8b949e; font-size: 13px; border: none;"); d->setWordWrap(true);

    layout->addWidget(t);
    layout->addWidget(v);
    layout->addWidget(d);
    return card;
}

void AnalysisPage::displayResults(const QJsonObject& r)
{
    auto* headerObj = new QWidget();
    auto* headerLayout = new QHBoxLayout(headerObj);
    auto* title = new QLabel(m_currentGameName);
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #ffffff; border: none; margin-bottom: 10px;");
    headerLayout->addWidget(title);
    m_detailsLayout->addWidget(headerObj);

    auto* statsFrame = new QFrame();
    statsFrame->setStyleSheet("background: #161b22; border-radius: 8px; border: 1px solid #30363d;");
    auto* statsLayout = new QHBoxLayout(statsFrame);
    statsLayout->setContentsMargins(20, 15, 20, 15);

    statsLayout->addWidget(createStatCard("FPS Médio", QString::number(r["fps_medio"].toDouble(), 'f', 0), "#ffffff"));
    statsLayout->addWidget(createStatCard("FPS Mín (1%)", QString::number(r["fps_min"].toDouble(), 'f', 0), "#f85149"));
    statsLayout->addWidget(createStatCard("Temp. Máx CPU", QString::number(r["temp_max_cpu"].toDouble(), 'f', 0)+"°C", "#ff7b72"));

    m_detailsLayout->addWidget(statsFrame);

    auto* chartTitle = new QLabel("Modelagem Matemática: f(Temperatura) = FPS");
    chartTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #58a6ff; margin-top: 20px; border: none;");
    m_detailsLayout->addWidget(chartTitle);

    double a = r["equacao_a"].toDouble();
    double b = r["equacao_b"].toDouble();
    double c = r["equacao_c"].toDouble();
    QString sinalB = (b >= 0) ? "+" : "";
    QString sinalC = (c >= 0) ? "+" : "";
    QString eqStr = QString("f(x) = %1x² %2%3x %4%5")
                        .arg(QString::number(a, 'f', 4)).arg(sinalB).arg(QString::number(b, 'f', 2)).arg(sinalC).arg(QString::number(c, 'f', 2));

    auto* eqLabel = new QLabel(eqStr);
    eqLabel->setStyleSheet("color: #238636; font-family: 'Consolas', monospace; font-weight: bold; font-size: 13px; border: none;");
    eqLabel->setAlignment(Qt::AlignCenter);
    m_detailsLayout->addWidget(eqLabel);

    m_chart = new StaticChartWidget();
    QList<double> fpsData, tempData;
    QFile file(m_currentCsvPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file); in.readLine();
        while (!in.atEnd()) {
            QStringList p = in.readLine().split(',');
            if (p.size() >= 3) {
                fpsData.append(p[1].toDouble());
                tempData.append(p[2].toDouble());
            }
        }
    }
    m_chart->loadData(fpsData, tempData);
    m_chart->setEquation(a, b, c);
    m_chart->setTangent(r["tangente_x"].toDouble(), r["tangente_y"].toDouble(), r["tangente_m"].toDouble());

    m_detailsLayout->addWidget(m_chart);

    auto* calcTitle = new QLabel("Engenharia Reversa de Performance");
    calcTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #d2a8ff; margin-top: 20px; border-bottom: 1px solid #30363d; padding-bottom: 5px; border-top: none; border-left: none; border-right: none;");
    m_detailsLayout->addWidget(calcTitle);

    double r2 = r["r_squared"].toDouble();
    QString r2Str = QString::number(r2 * 100, 'f', 1) + "%";
    QString r2Desc;
    QString r2Color;

    if (r2 > 0.6) {
        r2Desc = "<b>Alta Precisão:</b> Encontramos uma correlação matemática clara! O calor está afetando o desempenho seguindo o padrão da curva verde.";
        r2Color = "#238636";
    } else if (r2 >= 0.0) {
        r2Desc = "<b>Baixa Correlação:</b> Os dados estão dispersos. O desempenho oscilou por outros motivos (loading, menus) e não apenas pela temperatura.";
        r2Color = "#e3b341";
    } else {
        r2Str = "Inconclusivo";
        r2Desc = "<b>Sessão Estável / Sem Variação:</b> O hardware não foi estressado o suficiente. "
                 "A temperatura e o FPS ficaram constantes (linha reta). "
                 "Para ver o Cálculo em ação, teste um jogo pesado e desligue o V-Sync.";
        r2Color = "#8b949e";
    }
    m_detailsLayout->addWidget(createMathCard("Validade do Modelo Matemático (R²)", r2Str, r2Desc, r2Color));

    double carga = r["carga_termica_cpu"].toDouble();
    double tvm = r["temp_tvm_cpu"].toDouble();
    m_detailsLayout->addWidget(createMathCard(
        "Carga Térmica & TVM (Integral)",
        QString::number(carga, 'f', 0) + " Gs",
        QString("<b>∫ f(t) dt:</b> Estresse térmico acumulado.<br><b>TVM:</b> O hardware se comportou como se estivesse a <b>%1°C</b> constantes.").arg(QString::number(tvm, 'f', 1)),
        "#d2a8ff"));

    double sensibilidade = r["tangente_m"].toDouble();
    QString sensVal = QString::number(sensibilidade, 'f', 2) + " FPS/°C";
    QString sensCor = (sensibilidade < -0.5) ? "#f85149" : "#79c0ff";
    m_detailsLayout->addWidget(createMathCard(
        "Sensibilidade no Pico (Derivada f')",
        sensVal,
        "<b>Inclinação da Tangente (Reta Roxa):</b> Taxa de variação instantânea no pico de calor. Se negativo, indica perda ativa de FPS por grau.",
        sensCor));

    double aceleracao = r["aceleracao_queda"].toDouble();
    QString acelVal = QString::number(aceleracao, 'f', 4);
    QString acelCor = (aceleracao < -0.001) ? "#ff7b72" : "#58a6ff";
    m_detailsLayout->addWidget(createMathCard(
        "Aceleração (Derivada f'')",
        acelVal,
        "<b>Concavidade:</b> Se negativo, a queda de desempenho acelera conforme esquenta (efeito bola de neve).",
        acelCor));

    double previsao = r["previsao_fps_100c"].toDouble();
    m_detailsLayout->addWidget(createMathCard(
        "Extrapolação (Limite T→100°C)",
        QString::number(previsao, 'f', 0) + " FPS",
        "<b>Previsão Matemática:</b> Desempenho estimado caso a temperatura atingisse o ponto de ebulição (limite físico).",
        "#e3b341"));

    m_detailsLayout->addStretch();
}
