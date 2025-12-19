<p align="center">
    <img src="extras/banner/banner_transparent.png" width="780">
</p>

<p align="center">
    <a href="https://github.com/Tapawingo/TrenchKit/releases">
        <img src="https://img.shields.io/github/v/release/Tapawingo/TrenchKit?style=flat-square" alt="Releases">
    </a>
    <a>
    <img src="https://img.shields.io/github/repo-size/Tapawingo/TrenchKit?style=flat-square" alt="repo Size">
    </a>
    <a href="https://github.com/Tapawingo/TrenchKit/issues" alt="Issue Tracker">
        <img src="https://img.shields.io/github/issues-raw/Tapawingo/TrenchKit?style=flat-square">
    </a>
    <a href="https://github.com/Tapawingo/TrenchKit/blob/master/LICENSE.md">
        <img src="https://img.shields.io/github/license/Tapawingo/TrenchKit?style=flat-square" alt="License">
    </a>
</p>

<p align="center">
    <strong>A small mod manager for Foxhole built with C++, Qt6.10, and CMake.</strong><br/>
    <sup>Not affiliated with Siege Camp / Foxhole. This is a community tool.</sup>
</p>


## Goals
- Make enabling/disabling mods simple
- Streamline mod installation and updates (using nexusmod's API)
- Keep changes transparent
- Be safe by default (backups, restore)

## Planned features (initial)
- Detect Foxhole install path
- List installed mods from a mods folder
- Enable/disable mods
- Profiles / presets
- Play with/without mods
- Install from nexus
- Update from nexus

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
**TrenchKit** is licensed under the MIT License ([MIT](LICENSE.md))

## Contributing
See [`CONTRIBUTING.md`](CONTRIBUTING.md).

## Acknowledgements
Foxhole is a trademark of its respective owners.