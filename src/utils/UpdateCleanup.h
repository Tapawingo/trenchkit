#pragma once

#include <QString>

class UpdateCleanup {
public:
    static void run();
    static void run(const QString &appDir);
};
