#ifndef WINDOWHELPERS_H
#define WINDOWHELPERS_H

#include <QString>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// A struct para passar dados para o callback
struct EnumData {
    DWORD processId;
    HWND bestHwnd;
    int bestTitleLength;
};

inline BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam)
{
    EnumData* data = reinterpret_cast<EnumData*>(lParam);
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);

    if (data->processId == processId && IsWindowVisible(hwnd)) {
        int length = GetWindowTextLength(hwnd);
        if (length > data->bestTitleLength) {
            wchar_t title[256];
            GetWindowTextW(hwnd, title, 256);
            QString qTitle = QString::fromWCharArray(title);
            // Filtra janelas indesejadas que alguns jogos criam
            if (qTitle != "D3DProxyWindow" && !qTitle.contains("NVIDIA") && !qTitle.contains("AMD") && qTitle.length() > 3) {
                data->bestHwnd = hwnd;
                data->bestTitleLength = length;
            }
        }
    }
    return TRUE;
}

#endif // WINDOWHELPERS_H
