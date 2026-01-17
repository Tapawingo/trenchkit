#include <QApplication>
#include "MainWindow.h"
#include "core/utils/UpdateCleanup.h"
#include "core/utils/Logger.h"
#include <QTimer>
#include <QCoreApplication>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
int main(int argc, char *argv[]);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main(__argc, __argv);
}
#endif

int main(int argc, char *argv[]) {
#ifdef _WIN32
    // Create named mutex to signal app is running (used by updater to wait for exit)
    HANDLE hMutex = CreateMutexW(nullptr, FALSE, L"Global\\TrenchKitRunning");
#endif

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("TrenchKit"));
    QCoreApplication::setApplicationName(QStringLiteral("TrenchKit"));

    Logger::instance().initialize();
    qInstallMessageHandler(Logger::messageHandler);

    qInfo() << "TrenchKit version:" << TRENCHKIT_VERSION;
    qInfo() << "Qt version:" << QT_VERSION_STR;
    qInfo() << "Log directory:" << Logger::instance().logDirectory();

    // Set application icon (for taskbar, alt-tab, etc.)
    app.setWindowIcon(QIcon(":/icon.png"));

    UpdateCleanup::run();

    MainWindow w;
    w.show();

    if (app.arguments().contains("--smoke-test")) {
        QTimer::singleShot(200, &app, &QCoreApplication::quit);
    }

    int result = app.exec();

#ifdef _WIN32
    if (hMutex) {
        CloseHandle(hMutex);
    }
#endif

    Logger::instance().shutdown();
    return result;
}
