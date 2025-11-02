#ifndef APPCONSTANTS_H
#define APPCONSTANTS_H

#include <QString>

namespace AppConfig {
// Chaves de Hardware
const QString CPU_KEY = "CPU";
const QString GPU_KEY = "GPU";
const QString MB_KEY = "MOTHERBOARD";
const QString STORAGE_KEY_PREFIX = "STORAGE_";

// Chaves de Configurações
const QString SETTING_PARTICLES_ENABLED = "particles/enabled";
const QString SETTING_REPORTS_ENABLED = "reports/enabled";

// Chaves de Configuração do Overlay
const QString SETTING_OVERLAY_ENABLED = "overlay/enabled";
const QString SETTING_OVERLAY_POSITION = "overlay/position";
const QString SETTING_OVERLAY_STYLE = "overlay/style";
const QString SETTING_OVERLAY_SHOW_CPU_TEMP = "overlay/showCpuTemp";
const QString SETTING_OVERLAY_SHOW_CPU_USAGE = "overlay/showCpuUsage";
const QString SETTING_OVERLAY_SHOW_CPU_CORES = "overlay/showCpuCores";
const QString SETTING_OVERLAY_SHOW_CPU_POWER = "overlay/showCpuPower";
const QString SETTING_OVERLAY_SHOW_CPU_CLOCK = "overlay/showCpuClock";
const QString SETTING_OVERLAY_SHOW_GPU_TEMP = "overlay/showGpuTemp";
const QString SETTING_OVERLAY_SHOW_GPU_USAGE = "overlay/showGpuUsage";
const QString SETTING_OVERLAY_SHOW_GPU_POWER = "overlay/showGpuPower";
const QString SETTING_OVERLAY_SHOW_GPU_CLOCK = "overlay/showGpuClock";
const QString SETTING_OVERLAY_SHOW_RAM_USAGE = "overlay/showRamUsage";
const QString SETTING_OVERLAY_SHOW_FANS = "overlay/showFans";
const QString SETTING_OVERLAY_SHOW_MB_TEMP = "overlay/showMbTemp";
const QString SETTING_OVERLAY_SHOW_STORAGE_TEMP = "overlay/showStorageTemp";
const QString SETTING_OVERLAY_SHOW_AVG_FPS = "overlay/showAvgFps";
const QString SETTING_OVERLAY_SHOW_MIN_FPS = "overlay/showMinFps";
const QString SETTING_OVERLAY_SHOW_MAX_FPS = "overlay/showMaxFps";


// Conteúdo SVG dos Ícones
const QString ICON_CPU_SVG = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#94a3b8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="4" y="4" width="16" height="16" rx="2" ry="2"></rect><rect x="9" y="9" width="6" height="6"></rect><line x1="9" y1="1" x2="9" y2="4"></line><line x1="15" y1="1" x2="15" y2="4"></line><line x1="9" y1="20" x2="9" y2="23"></line><line x1="15" y1="20" x2="15" y2="23"></line><line x1="20" y1="9" x2="23" y2="9"></line><line x1="20" y1="14" x2="23" y2="14"></line><line x1="1" y1="9" x2="4" y2="9"></line><line x1="1" y1="14" x2="4" y2="14"></line></svg>)";
const QString ICON_GPU_SVG = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#94a3b8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="1" y="4" width="22" height="16" rx="2" ry="2"></rect><line x1="1" y1="12" x2="23" y2="12"></line></svg>)";
const QString ICON_MB_SVG = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#94a3b8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="2" y="2" width="20" height="20" rx="2.18" ry="2.18"></rect><line x1="7" y1="12" x2="7" y2="12.01"></line><line x1="12" y1="7" x2="12" y2="7.01"></line><line x1="12" y1="12" x2="12" y2="12.01"></line><line x1="12" y1="17" x2="12" y2="17.01"></line><line x1="17" y1="12" x2="17" y2="12.01"></line></svg>)";
const QString ICON_STORAGE_SVG = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#94a3b8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><ellipse cx="12" cy="5" rx="9" ry="3"></ellipse><path d="M21 12c0 1.66-4 3-9 3s-9-1.34-9-3"></path><path d="M3 5v14c0 1.66 4 3 9 3s9-1.34 9-3V5"></path></svg>)";
const QString ICON_FAN_SVG = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#94a3b8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M9.59 4.59A2 2 0 1 1 11 8H2m10.59 11.41A2 2 0 1 0 11 16H2m15.73-8.27A2.5 2.5 0 1 1 19.5 12H2"></path></svg>)";
const QString ICON_TEMP_SVG = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#94a3b8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"></path></svg>)";
const QString ICON_USAGE_SVG = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#94a3b8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M2 12s3-7 10-7 10 7 10 7-3 7-10 7-10-7-10-7Z"></path><circle cx="12" cy="12" r="3"></circle></svg>)";
const QString ICON_POWER_SVG = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#94a3b8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M13 2 3 14h9l-1 8 10-12h-9l1-8z"></path></svg>)";
const QString ICON_CLOCK_SVG = R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#94a3b8" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"></circle><polyline points="12 6 12 12 16 14"></polyline></svg>)";
}

#endif // APPCONSTANTS_H
