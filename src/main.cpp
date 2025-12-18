#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application icon (for taskbar, alt-tab, etc.)
    app.setWindowIcon(QIcon(":/icon.png"));

    MainWindow w;
    w.show();
    return app.exec();
}
