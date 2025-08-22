#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QThread>
#include <QMessageBox> // Para a mensagem de erro

#include <comdef.h> // Inclua as bibliotecas do COM
#include <Wbemidl.h>

// Adicione os pragma comments aqui
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "comsuppw.lib")

int main(int argc, char *argv[])
{
    // A inicialização correta para threads de UI (como o thread principal do Qt)
    HRESULT hres = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if(FAILED(hres))
    {
        QMessageBox::critical(nullptr, "Erro Crítico", "Falha ao inicializar o COM. O programa não pode continuar.");
        return -1; // Sai do programa
    }

    QApplication a(argc, argv);

    QPixmap pixmap(":/logo-site.png");
    QSplashScreen splash(pixmap);
    splash.show();

    QThread::sleep(3);

    MainWindow w;
    splash.finish(&w);
    w.show();

    int result = a.exec();

    // Desinicialização do COM no final
    CoUninitialize();

    return result;
}