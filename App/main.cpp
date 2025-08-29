#include "mainwindow.h"
#include "splashscreen.h"
#include <QApplication>
#include <QTimer>
#include <QSslSocket>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // VERIFICAÇÃO CRÍTICA E MENSAGEM DE ERRO MELHORADA
    if (!QSslSocket::supportsSsl()) {
        QMessageBox::critical(nullptr, "Erro de Rede Crítico - OpenSSL Faltando",
                              "O aplicativo não conseguiu encontrar as bibliotecas OpenSSL necessárias para conexões seguras (HTTPS).\n\n"
                              "Como resultado, a busca online por nomes e capas de jogos não funcionará.\n\n"
                              "**COMO RESOLVER:**\n"
                              "1. Encontre a pasta de instalação do Qt no seu computador.\n"
                              "2. Navegue até a pasta 'bin' do seu compilador (ex: C:\\Qt\\6.7.0\\mingw_64\\bin).\n"
                              "3. Copie os arquivos 'libcrypto-3-x64.dll' e 'libssl-3-x64.dll'.\n"
                              "4. Cole esses dois arquivos na mesma pasta onde o 'LagZero.exe' está localizado.",
                              QMessageBox::Ok);
    }

    SplashScreen splash;
    splash.show();

    MainWindow w;

    QTimer::singleShot(3000, [&](){
        splash.close();
        w.show();
    });

    return a.exec();
}
