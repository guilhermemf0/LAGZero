#include "analysispage.h"
#include "databasemanager.h"
#include "appconstants.h" // Certifique-se de ter este arquivo para os ícones
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
#include <QMessageBox> // Necessário para confirmação de exclusão

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

    // --- COLUNA DA ESQUERDA (LISTA) ---
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

    // Botão Deletar
    auto* deleteBtn = new QPushButton("Deletar Sessão Selecionada");
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setStyleSheet("QPushButton { background-color: #da3633; color: white; border-radius: 6px; padding: 8px; font-weight: bold; border: none; }"
                             "QPushButton:hover { background-color: #b62324; }");
    connect(deleteBtn, &QPushButton::clicked, this, [this](){
        QListWidgetItem* item = m_sessionList->currentItem();
        if (!item) return;

        // Confirmação
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirmar Exclusão", "Tem certeza que deseja apagar esta análise e seus dados?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            int id = item->data(Qt::UserRole + 2).toInt(); // Recupera o ID salvo
            if (DatabaseManager::instance().deleteSession(id)) {
                refreshSessions(); // Atualiza a lista

                // Limpa a tela da direita se a sessão deletada era a atual
                // (Simplesmente limpamos tudo para garantir)
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

    // --- COLUNA DA DIREITA (DETALHES) ---
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
        // Formata o texto do item da lista
        QString itemText = QString("%1\n%2 • %3").arg(s.gameName, s.startTime, s.duration);
        auto* item = new QListWidgetItem(itemText);

        // Salva dados ocultos no item para uso posterior
        item->setData(Qt::UserRole, s.reportPath);
        item->setData(Qt::UserRole + 1, s.gameName);
        item->setData(Qt::UserRole + 2, s.id); // Salva o ID para deletar

        m_sessionList->addItem(item);
    }
}

void AnalysisPage::onSessionSelected(QListWidgetItem* item)
{
    QString csvPath = item->data(Qt::UserRole).toString();
    m_currentGameName = item->data(Qt::UserRole + 1).toString();
    m_currentCsvPath = csvPath;

    // Limpa a área de detalhes
    QLayoutItem* child;
    while ((child = m_detailsLayout->takeAt(0)) != 0) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    // Mostra mensagem de carregamento
    auto* loading = new QLabel("Executando Análise Matemática Avançada...");
    loading->setStyleSheet("color: #58a6ff; font-weight: bold; font-size: 16px; margin-top: 50px; border: none;");
    loading->setAlignment(Qt::AlignCenter);
    m_detailsLayout->addWidget(loading);

    // Chama o Python
    QString scriptPath = QCoreApplication::applicationDirPath() + "/analise_sessao.py";
    m_pythonProcess->start("python", QStringList() << scriptPath << csvPath);
}

void AnalysisPage::onPythonFinished(int, QProcess::ExitStatus)
{
    // Remove o loading
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

// Função auxiliar para criar os cartões de estatística simples
QWidget* createStatCard(const QString& label, const QString& value, const QString& color) {
    auto* w = new QWidget();
    auto* l = new QVBoxLayout(w);
    l->setContentsMargins(0,0,0,0); l->setSpacing(2);
    auto* lbl = new QLabel(label); lbl->setStyleSheet("color: #8b949e; font-size: 12px; border: none;");
    auto* val = new QLabel(value); val->setStyleSheet(QString("color: %1; font-size: 18px; font-weight: bold; border: none;").arg(color));
    l->addWidget(lbl); l->addWidget(val);
    return w;
}

// Função auxiliar para criar os cartões de explicação matemática (com borda colorida na esquerda)
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
    // 1. Título do Jogo
    auto* title = new QLabel(m_currentGameName);
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #ffffff; border: none; margin-bottom: 10px;");
    m_detailsLayout->addWidget(title);

    // 2. Resumo Geral (Stats)
    auto* statsFrame = new QFrame();
    statsFrame->setStyleSheet("background: #161b22; border-radius: 8px; border: 1px solid #30363d;");
    auto* statsLayout = new QHBoxLayout(statsFrame);
    statsLayout->setContentsMargins(20, 15, 20, 15);

    statsLayout->addWidget(createStatCard("FPS Médio", QString::number(r["fps_medio"].toDouble(), 'f', 0), "#ffffff"));
    statsLayout->addWidget(createStatCard("FPS Mín (1%)", QString::number(r["fps_min"].toDouble(), 'f', 0), "#f85149")); // Vermelho
    statsLayout->addWidget(createStatCard("Temp. Máx CPU", QString::number(r["temp_max_cpu"].toDouble(), 'f', 0)+"°C", "#ff7b72"));

    m_detailsLayout->addWidget(statsFrame);

    // 3. Título e Gráfico
    auto* chartTitle = new QLabel("Correlação: Temperatura (X) vs. Desempenho (Y)");
    chartTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #58a6ff; margin-top: 20px; border: none;");
    m_detailsLayout->addWidget(chartTitle);

    // --- EXIBIR A EQUAÇÃO ---
    double a = r["equacao_a"].toDouble();
    double b = r["equacao_b"].toDouble();
    double c = r["equacao_c"].toDouble();

    QString sinalB = (b >= 0) ? "+" : "";
    QString sinalC = (c >= 0) ? "+" : "";

    QString eqStr = QString("Modelo Matemático: f(x) = %1x² %2%3x %4%5")
                        .arg(QString::number(a, 'f', 4))
                        .arg(sinalB).arg(QString::number(b, 'f', 2))
                        .arg(sinalC).arg(QString::number(c, 'f', 2));

    auto* eqLabel = new QLabel(eqStr);
    eqLabel->setStyleSheet("color: #238636; font-family: 'Consolas', 'Courier New', monospace; font-weight: bold; font-size: 13px; border: none;");
    eqLabel->setAlignment(Qt::AlignCenter);
    m_detailsLayout->addWidget(eqLabel);
    // ------------------------

    m_chart = new StaticChartWidget();

    // Carregar dados do CSV para os pontos
    QList<double> fpsData, tempData;
    QFile file(m_currentCsvPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file); in.readLine(); // Pula header
        while (!in.atEnd()) {
            QStringList p = in.readLine().split(',');
            if (p.size() >= 3) {
                fpsData.append(p[1].toDouble());
                tempData.append(p[2].toDouble());
            }
        }
    }
    m_chart->loadData(fpsData, tempData);
    // Passa a equação para o gráfico desenhar a linha
    m_chart->setEquation(a, b, c);

    m_detailsLayout->addWidget(m_chart);

    // 4. Explicações Matemáticas (Cards)

    // --- INTEGRAL ---
    double carga = r["carga_termica_cpu"].toDouble();
    QString integralDesc =
        "<b>O que é?</b> A Integral Definida calcula a área total sob a curva de temperatura no gráfico Tempo x Temp.<br>"
        "<b>Interpretação:</b> Este valor representa o 'dano acumulado' ou estresse total que o processador sofreu nesta sessão. "
        "Valores muito altos indicam que o sistema de refrigeração foi exigido por muito tempo.";

    m_detailsLayout->addWidget(createMathCard("Estresse Térmico Total (Integral)",
                                              QString::number(carga, 'f', 0) + " Grau-segundos",
                                              integralDesc, "#d2a8ff")); // Roxo

    // --- DERIVADA ---
    double gargalo = r["gargalo_cpu"].toDouble();
    QString gargaloVal;
    QString derivDesc;

    if (gargalo > 0) {
        gargaloVal = QString("Risco em %1 °C").arg(QString::number(gargalo, 'f', 1));
        derivDesc = "<b>O que é?</b> Usamos a Derivada para encontrar o ponto onde a inclinação da curva de performance se torna zero (máximo local).<br>"
                    "<b>Alerta:</b> Detectamos que, ao atingir essa temperatura, o desempenho (FPS) parou de subir ou começou a cair. "
                    "Isso sugere Thermal Throttling.";
    } else {
        gargaloVal = "Estável (Sem Gargalo)";
        derivDesc = "<b>O que é?</b> Usamos a Derivada para analisar a taxa de variação do desempenho em relação à temperatura.<br>"
                    "<b>Diagnóstico:</b> A derivada se manteve positiva ou estável. O aumento de temperatura não causou queda de FPS nesta sessão.";
    }

    m_detailsLayout->addWidget(createMathCard("Ponto de Ruptura (Derivada)",
                                              gargaloVal,
                                              derivDesc, "#ff7b72")); // Vermelho/Laranja

    // --- MÉDIA ---
    double media = r["temp_media_cpu"].toDouble();
    m_detailsLayout->addWidget(createMathCard("Temperatura Média",
                                              QString::number(media, 'f', 1) + " °C",
                                              "Média aritmética simples da temperatura durante toda a sessão.", "#79c0ff")); // Azul

    m_detailsLayout->addStretch();
}
