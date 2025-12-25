#include <QApplication>
#include "MainWindow.h"
#include "utils/UpdateCleanup.h"
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
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("TrenchKit"));
    QCoreApplication::setApplicationName(QStringLiteral("TrenchKit"));

    // Set application icon (for taskbar, alt-tab, etc.)
    app.setWindowIcon(QIcon(":/icon.png"));

    UpdateCleanup::run();

    MainWindow w;
    w.show();

    if (app.arguments().contains("--smoke-test")) {
        QTimer::singleShot(200, &app, &QCoreApplication::quit);
    }
    return app.exec();
}
