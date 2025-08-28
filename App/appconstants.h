#ifndef APPCONSTANTS_H
#define APPCONSTANTS_H

#include <QString>

namespace AppConfig {
// Chaves de Hardware
const QString CPU_KEY = "CPU";
const QString GPU_KEY = "GPU";
const QString MB_KEY = "MOTHERBOARD"; // Placa-mãe de volta
const QString STORAGE_KEY_PREFIX = "STORAGE_";

// Chaves de Configurações
const QString SETTING_PARTICLES_ENABLED = "particles/enabled";
const QString SETTING_REPORTS_ENABLED = "reports/enabled";

// Conteúdo SVG dos Ícones
const QString ICON_CPU_SVG = R"(<svg ...></svg>)"; // (Conteúdo SVG omitido por brevidade)
const QString ICON_GPU_SVG = R"(<svg ...></svg>)";
const QString ICON_MB_SVG = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="2" y="2" width="20" height="20" rx="2.18" ry="2.18"></rect><line x1="7" y1="12" x2="7" y2="12.01"></line><line x1="12" y1="7" x2="12" y2="7.01"></line><line x1="12" y1="12" x2="12" y2="12.01"></line><line x1="12" y1="17" x2="12" y2="17.01"></line><line x1="17" y1="12" x2="17" y2="12.01"></line></svg>)";
const QString ICON_STORAGE_SVG = R"(<svg ...></svg>)";
}

#endif // APPCONSTANTS_H
