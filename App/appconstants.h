#ifndef APPCONSTANTS_H
#define APPCONSTANTS_H

#include <QString>

// Namespace para guardar constantes globais e chaves usadas no aplicativo
namespace AppConfig {
    // Chaves de Hardware (usadas para identificar os componentes)
    const QString CPU_KEY = "CPU";
    const QString GPU_KEY = "GPU";
    const QString MB_KEY = "MOTHERBOARD";
    const QString STORAGE_KEY_PREFIX = "STORAGE_";

    // Chaves de Configurações (usadas no QSettings)
    const QString SETTING_PARTICLES_ENABLED = "particles/enabled";
    const QString SETTING_REPORTS_ENABLED = "reports/enabled";

    // Ícones
    const QString ICON_CPU = ":/icons/cpu.svg";
    const QString ICON_GPU = ":/icons/gpu.svg";
    const QString ICON_MB = ":/icons/motherboard.svg";
    const QString ICON_STORAGE = ":/icons/storage.svg";
    const QString ICON_FPS = ":/icons/fps.svg";
}

#endif // APPCONSTANTS_H
