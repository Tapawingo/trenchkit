cmake_minimum_required(VERSION 3.24)

project(TrenchKit
  VERSION 1.1.3
  LANGUAGES CXX
)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets Network Concurrent Test WebSockets)

# Fetch zip library for archive extraction (MIT, zero dependencies)
include(FetchContent)
FetchContent_Declare(
  zip
  GIT_REPOSITORY https://github.com/kuba--/zip.git
  GIT_TAG v0.3.2
)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(ZIP_STATIC ON)
FetchContent_MakeAvailable(zip)

set_target_properties(zip PROPERTIES
  AUTOMOC OFF
  AUTOUIC OFF
  AUTORCC OFF
)

target_compile_options(zip PRIVATE
  $<$<AND:$<COMPILE_LANGUAGE:C>,$<NOT:$<C_COMPILER_ID:MSVC>>>:-Wno-error>
)

# Fetch libarchive for multi-format archive extraction (BSD license)
FetchContent_Declare(
  libarchive
  GIT_REPOSITORY https://github.com/libarchive/libarchive.git
  GIT_TAG v3.7.4
)

set(ENABLE_TAR ON)
set(ENABLE_CPIO OFF)
set(ENABLE_CAT OFF)
set(ENABLE_TEST OFF)
set(ENABLE_INSTALL OFF)
set(ENABLE_STATIC ON)
set(ENABLE_SHARED OFF)
set(ENABLE_ACL OFF)
set(ENABLE_XATTR OFF)

FetchContent_MakeAvailable(libarchive)

set_target_properties(archive_static PROPERTIES
  AUTOMOC OFF
  AUTOUIC OFF
  AUTORCC OFF
)

qt_standard_project_setup()

add_subdirectory(updater)
add_subdirectory(src)
add_subdirectory(tests)
