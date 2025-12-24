#include <QApplication>
#include "MainWindow.h"
#include "utils/UpdateCleanup.h"
#include <QTimer>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

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
