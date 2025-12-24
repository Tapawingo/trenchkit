#include <filesystem>
#include <string>
#include <vector>
#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#include <tlhelp32.h>
#endif

namespace fs = std::filesystem;

struct InstallArgs {
    bool install = false;
    fs::path appDir;
    fs::path newDir;
    std::wstring exeName;
};

static bool equalsIgnoreCase(const std::wstring &a, const std::wstring &b) {
#if defined(_WIN32)
    return _wcsicmp(a.c_str(), b.c_str()) == 0;
#else
    return a == b;
#endif
}

static bool parseArgs(int argc, char **argv, InstallArgs &out) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--install") {
            out.install = true;
        } else if (arg == "--app-dir" && i + 1 < argc) {
            out.appDir = fs::path(argv[++i]);
        } else if (arg == "--new-dir" && i + 1 < argc) {
            out.newDir = fs::path(argv[++i]);
        } else if (arg == "--exe-name" && i + 1 < argc) {
            const std::string exe = argv[++i];
            out.exeName = std::wstring(exe.begin(), exe.end());
        }
    }
    return out.install && !out.appDir.empty() && !out.newDir.empty() && !out.exeName.empty();
}

#if defined(_WIN32)
static bool isProcessRunning(const std::wstring &exeName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(PROCESSENTRY32W);

    bool found = false;
    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (equalsIgnoreCase(entry.szExeFile, exeName)) {
                found = true;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return found;
}

static void waitForProcessExit(const std::wstring &exeName) {
    while (isProcessRunning(exeName)) {
        Sleep(200);
    }
}
#endif

static bool copyRecursive(const fs::path &from, const fs::path &to) {
    std::error_code ec;
    fs::create_directories(to, ec);

    for (const auto &entry : fs::recursive_directory_iterator(from)) {
        const auto relPath = fs::relative(entry.path(), from, ec);
        if (ec) {
            return false;
        }
        const fs::path destPath = to / relPath;
        if (entry.is_directory()) {
            fs::create_directories(destPath, ec);
            if (ec) {
                return false;
            }
            continue;
        }
        if (entry.is_regular_file()) {
            fs::create_directories(destPath.parent_path(), ec);
            if (ec) {
                return false;
            }
            fs::copy_file(entry.path(), destPath, fs::copy_options::overwrite_existing, ec);
            if (ec) {
                return false;
            }
        }
    }

    return true;
}

static void removeOldAppFiles(const fs::path &appDir) {
    std::error_code ec;
    for (const auto &entry : fs::directory_iterator(appDir)) {
        const fs::path name = entry.path().filename();
#if defined(_WIN32)
        const std::wstring entryName = name.wstring();
        if (equalsIgnoreCase(entryName, L"updater.exe")) {
            continue;
        }
        if (equalsIgnoreCase(entryName, L"updates")) {
            continue;
        }
#else
        if (name == "updater" || name == "updates") {
            continue;
        }
#endif
        fs::remove_all(entry.path(), ec);
    }
}

#if defined(_WIN32)
static bool launchApp(const fs::path &appDir, const std::wstring &exeName) {
    const fs::path exePath = appDir / exeName;
    const HINSTANCE result = ShellExecuteW(
        nullptr,
        L"open",
        exePath.c_str(),
        nullptr,
        appDir.c_str(),
        SW_SHOWNORMAL);
    return reinterpret_cast<intptr_t>(result) > 32;
}
#endif

int main(int argc, char **argv) {
    InstallArgs args;
    if (!parseArgs(argc, argv, args)) {
        std::cerr << "Usage: updater --install --app-dir <dir> --new-dir <dir> --exe-name <name>\n";
        return 1;
    }

#if defined(_WIN32)
    waitForProcessExit(args.exeName);
#else
    std::cerr << "TODO: implement process wait for non-Windows platforms.\n";
#endif

    if (!fs::exists(args.newDir)) {
        std::cerr << "New version directory not found.\n";
        return 1;
    }

    removeOldAppFiles(args.appDir);

    if (!copyRecursive(args.newDir, args.appDir)) {
        std::cerr << "Failed to copy new version files.\n";
        return 1;
    }

#if defined(_WIN32)
    if (!launchApp(args.appDir, args.exeName)) {
        std::cerr << "Failed to relaunch application.\n";
        return 1;
    }
#else
    std::cerr << "TODO: relaunch for non-Windows platforms.\n";
#endif

    return 0;
}
