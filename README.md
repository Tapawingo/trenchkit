# Foxhole Mod Manager (Qt)

A small mod manager for **Foxhole** built with **C++**, **Qt**, and **CMake**.

> Not affiliated with Siege Camp / Foxhole. This is a community tool.

## Goals
- Make enabling/disabling mods simple
- Keep changes transparent (show what files get copied/overwritten)
- Be safe by default (backups, restore, validation)

## Planned features (initial)
- Detect Foxhole install path (manual + remembered)
- List installed mods from a mods folder
- Enable/disable mods (copy or link strategy)
- Profiles / presets
- Logs + “dry run” preview

## Requirements
- A C++20 compiler toolchain
- CMake 3.24+
- Ninja
- Qt 6.2+ (Widgets)

## Project layout
```text
.
├─ CMakeLists.txt
├─ src/
│  ├─ CMakeLists.txt
│  ├─ main.cpp
│  ├─ MainWindow.h
│  ├─ MainWindow.cpp
│  ├─ MainWindow.ui
│  └─ resources.qrc
├─ README.md
└─ CONTRIBUTING.md
```

## Build (Ninja)

### 1) Install dependencies
- Install **Qt 6** (development files + CMake config packages)
- Install **Ninja**
- Install a C++ compiler (MSVC, clang, or gcc)

### 2) Configure (Debug)
```sh
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

### 3) Build
```sh
cmake --build build/debug
```

### 4) Configure (Release)
```sh
cmake -S . -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### 5) Build
```sh
cmake --build build/release
```

## If CMake can’t find Qt
Provide a hint to your Qt installation.

### Option A: `CMAKE_PREFIX_PATH`
```sh
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="</path/to/Qt>"
```

### Option B: `Qt6_DIR`
```sh
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DQt6_DIR="</path/to/Qt>/lib/cmake/Qt6"
```

### Option C: vcpkg toolchain
```sh
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE="</path/to/vcpkg>/scripts/buildsystems/vcpkg.cmake"
```

## Running
Run the produced executable from the build output directory.

If you want to run the app outside your build environment, you may need to deploy Qt runtime libraries.
On some platforms this is handled automatically by your packaging system; on others you’ll use Qt’s deployment tools.

## License
- This project: MIT (add a `LICENSE` file)
- Qt: LGPL / GPL / Commercial depending on the Qt modules and how you link/distribute

If you distribute binaries, ensure you follow the license obligations for Qt and any other third-party dependencies.

## Contributing
See `CONTRIBUTING.md`.

## Acknowledgements
Foxhole is a trademark of its respective owners.
