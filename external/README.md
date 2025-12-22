# External Dependencies

This directory contains third-party libraries integrated as git submodules.

## QuaZip

**Version**: Latest from main branch
**Repository**: https://github.com/stachenov/quazip.git
**License**: LGPL 2.1 with static linking exception
**Purpose**: ZIP archive extraction for mod files

QuaZip is built as a static library and statically linked into TrenchKit. The LGPL 2.1 license includes a static linking exception, allowing TrenchKit to remain under the MIT license.

### Building

QuaZip is automatically built when you build TrenchKit. The CMake configuration:
- Sets `QUAZIP_QT_MAJOR_VERSION=6` to build for Qt6
- Sets `QUAZIP_USE_QT_ZLIB=ON` to use Qt's bundled zlib (no system zlib required)
- Sets `BUILD_SHARED_LIBS=OFF` to create a static library

### Cloning

If you didn't clone with `--recurse-submodules`, initialize the submodule:

```bash
git submodule update --init --recursive
```

### Updating

To update QuaZip to the latest version:

```bash
cd external/quazip
git pull origin master
cd ../..
git add external/quazip
git commit -m "Update QuaZip submodule"
```
