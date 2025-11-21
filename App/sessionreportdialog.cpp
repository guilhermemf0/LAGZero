#include "sessionreportdialog.h"
#include "appconstants.h" // Para os ícones SVG
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QScrollArea>
#include <QtSvg/QSvgRenderer>
#include <QPainter>

SessionReportDialog::SessionReportDialog(const QJsonObject& data, const QString& gameName, QWidget *parent)
    : QDialog(parent), m_data(data), m_gameName(gameName)
{
    setWindowTitle("Relatório de Análise Avançada (Cálculo)");
    resize(600, 700);
    setStyleSheet("QDialog { background-color: #05070d; color: #cbd5e1; font-family: 'Inter'; }");
    setupUi();
}

void SessionReportDialog::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    // Cabeçalho
    auto* titleLabel = new QLabel(QString("Análise de Sessão: %1").arg(m_gameName));
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; } QWidget { background: transparent; }");

    auto* contentWidget = new QWidget();
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(15);

    // --- SEÇÃO CPU ---
    auto* cpuTitle = new QLabel("Processador (CPU)");
    cpuTitle->setStyleSheet("font-size: 18px; font-weight: 600; color: #00d1ff; margin-top: 10px;");
    contentLayout->addWidget(cpuTitle);

    // 1. INTEGRAL (Carga Térmica)
    double integralCpu = m_data["carga_termica_cpu_gs"].toDouble();
    QString descIntegral = "<b>O que é isso?</b> Cálculo da área total sob o gráfico de temperatura.<br>"
                           "<b>O que significa?</b> Representa o 'acumulado' de calor que seu processador sofreu.<br>"
                           "Imagine como a 'quilometragem' térmica da sessão.<br>"
                           "<i>Matemática: ∫ T(t) dt (Integral Definida)</i>";
    contentLayout->addWidget(createCalculusCard("Carga Térmica (Integral)",
                                                QString::number(integralCpu, 'f', 0) + " Grau-segundos",
                                                descIntegral, AppConfig::ICON_TEMP_SVG));

    // 2. DERIVADA (Gargalo)
    double derivCpu = m_data["temp_gargalo_cpu_c"].toDouble();
    QString descDeriv = "<b>O que é isso?</b> Análise da taxa de variação (inclinação da curva).<br>"
                        "<b>O que significa?</b> Tentamos encontrar a temperatura exata onde o aumento de calor para de gerar mais FPS (ou começa a cair).<br>"
                        "Se detectado, é o seu 'limite térmico' prático.<br>"
                        "<i>Matemática: Encontrar T onde f'(T) = 0 (Derivada Nula)</i>";

    QString valorDeriv = (derivCpu > 0) ? QString::number(derivCpu, 'f', 1) + " °C" : "Estável (Sem Gargalo)";
    contentLayout->addWidget(createCalculusCard("Ponto de Gargalo (Derivada)",
                                                valorDeriv,
                                                descDeriv, AppConfig::ICON_CPU_SVG));

    // --- SEÇÃO GPU (Se houver dados) ---
    if (m_data["carga_termica_gpu_gs"].toDouble() > 0) {
        auto* gpuTitle = new QLabel("Placa de Vídeo (GPU)");
        gpuTitle->setStyleSheet("font-size: 18px; font-weight: 600; color: #ff7043; margin-top: 20px;");
        contentLayout->addWidget(gpuTitle);

        double integralGpu = m_data["carga_termica_gpu_gs"].toDouble();
        contentLayout->addWidget(createCalculusCard("Carga Térmica Total (Integral)",
                                                    QString::number(integralGpu, 'f', 1) + " Grau-segundos",
                                                    descIntegral, AppConfig::ICON_TEMP_SVG));

        double derivGpu = m_data["temp_gargalo_gpu_c"].toDouble();
        QString valorDerivGpu = (derivGpu > 0) ? QString::number(derivGpu, 'f', 1) + " °C" : "Não detectado (Estável)";
        contentLayout->addWidget(createCalculusCard("Ponto de Gargalo (Derivada)",
                                                    valorDerivGpu,
                                                    descDeriv, AppConfig::ICON_GPU_SVG));
    }

    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    auto* closeBtn = new QPushButton("Fechar Relatório");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton { background-color: #0085ff; color: white; border-radius: 6px; padding: 10px; font-weight: bold; } QPushButton:hover { background-color: #006bcccc; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn);
}

QWidget* SessionReportDialog::createCalculusCard(const QString& title, const QString& value, const QString& description, const QString& iconPath)
{
    auto* card = new QFrame();
    card->setStyleSheet("QFrame { background-color: #1e293b; border-radius: 12px; border: 1px solid #334155; }");
    auto* layout = new QHBoxLayout(card);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);

    // Ícone
    auto* iconLabel = new QLabel();
    iconLabel->setFixedSize(48, 48);
    QSvgRenderer renderer(iconPath.toUtf8());
    QPixmap pm(48, 48);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    renderer.render(&p);
    iconLabel->setPixmap(pm);
    layout->addWidget(iconLabel);

    // Textos
    auto* textLayout = new QVBoxLayout();
    auto* titleLbl = new QLabel(title);
    titleLbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #e2e8f0; border: none;");

    auto* valueLbl = new QLabel(value);
    valueLbl->setStyleSheet("font-size: 24px; font-weight: bold; color: #ffffff; border: none; margin: 5px 0;");

    auto* descLbl = new QLabel(description);
    descLbl->setStyleSheet("font-size: 13px; color: #94a3b8; border: none;");
    descLbl->setWordWrap(true);

    textLayout->addWidget(titleLbl);
    textLayout->addWidget(valueLbl);
    textLayout->addWidget(descLbl);

    layout->addLayout(textLayout);
    return card;
}
