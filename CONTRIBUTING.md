# Contributing

Thanks for helping improve this project!

## Ground rules
- Be respectful and constructive.
- Keep changes focused and easy to review.
- Prefer clarity over cleverness.

## How to contribute

### Bug reports
When opening an issue, please include:
- What you expected to happen
- What actually happened
- Steps to reproduce
- Logs/screenshots if relevant
- Your setup (OS, compiler/toolchain, Qt version, CMake version)

### Feature requests
Describe the user problem first, then the proposed solution.
If it touches packaging/licensing-sensitive areas (Qt modules, deployment, bundling), mention that upfront.

### Pull requests
- Fork the repo and create a feature branch: `feature/<name>` or `fix/<name>`
- Keep PRs small where possible
- Update docs if behavior changes
- Ensure it configures and builds with CMake + Ninja

## Build instructions (Ninja)

### Configure (Debug)
```sh
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

### Build
```sh
cmake --build build/debug
```

### Configure (Release)
```sh
cmake -S . -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### Build
```sh
cmake --build build/release
```

## If Qt isn’t found
Prefer one of these approaches:

### `CMAKE_PREFIX_PATH`
```sh
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="</path/to/Qt>"
```

### `Qt6_DIR`
```sh
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DQt6_DIR="</path/to/Qt>/lib/cmake/Qt6"
```

### vcpkg toolchain
```sh
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE="</path/to/vcpkg>/scripts/buildsystems/vcpkg.cmake"
```

## Code style
- Keep it consistent with existing code
- Prefer modern C++ (C++20), RAII, and Qt idioms
- Avoid unnecessary dependencies
- UI: keep the main window simple and responsive

## Commit messages
Use clear, descriptive messages, e.g.:
- `Fix mod list refresh when folder missing`
- `Add profile save/load`
- `Improve install path detection`

## Licensing notes
- Repository code is MIT.
- Avoid adding Qt modules that are GPL-only unless the project explicitly opts into GPL terms or uses a commercial Qt license.
- If you add third-party code, ensure its license is compatible and add attribution where required.

## By contributing
You agree that your contributions may be redistributed under this repository’s license (MIT).
