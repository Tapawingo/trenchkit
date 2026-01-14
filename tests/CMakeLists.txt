qt_add_executable(tests_utilities
  TestUtilities.cpp
  ${CMAKE_SOURCE_DIR}/src/core/utils/ArchiveExtractor.h
  ${CMAKE_SOURCE_DIR}/src/core/utils/ArchiveExtractor.cpp
  ${CMAKE_SOURCE_DIR}/src/core/utils/UpdateArchiveExtractor.h
  ${CMAKE_SOURCE_DIR}/src/core/utils/UpdateArchiveExtractor.cpp
  ${CMAKE_SOURCE_DIR}/src/core/utils/UpdateCleanup.h
  ${CMAKE_SOURCE_DIR}/src/core/utils/UpdateCleanup.cpp
  ${CMAKE_SOURCE_DIR}/src/core/services/UpdaterService.h
  ${CMAKE_SOURCE_DIR}/src/core/services/UpdaterService.cpp
)

target_include_directories(tests_utilities
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/_deps/zip-src/src
    ${CMAKE_BINARY_DIR}/_deps/libarchive-src/libarchive/.
)

target_compile_definitions(tests_utilities
  PRIVATE
    LIBARCHIVE_STATIC
)

target_link_libraries(tests_utilities
  PRIVATE
    Qt6::Test
    Qt6::Network
    Qt6::Concurrent
    zip
    archive_static
)

add_test(NAME tests_utilities COMMAND tests_utilities)

if(WIN32)
  # Copy zip DLL if it exists
  set(ZIP_DLL_PATH "${CMAKE_BINARY_DIR}/_deps/zip-build/libzip.dll")
  if(EXISTS "${ZIP_DLL_PATH}")
    add_custom_command(TARGET tests_utilities POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ZIP_DLL_PATH}"
        $<TARGET_FILE_DIR:tests_utilities>
      COMMENT "Copying libzip.dll for tests_utilities"
    )
  endif()

  set(QT_BIN_DIR "${Qt6_DIR}/../../../bin")
  set(QT_TEST_DLLS "")
  foreach(QT_DLL_NAME
          Qt6Core.dll
          Qt6Test.dll
          Qt6Network.dll
          Qt6Concurrent.dll)
    if(EXISTS "${QT_BIN_DIR}/${QT_DLL_NAME}")
      list(APPEND QT_TEST_DLLS "${QT_BIN_DIR}/${QT_DLL_NAME}")
    endif()
  endforeach()
  if(QT_TEST_DLLS)
    add_custom_command(TARGET tests_utilities POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QT_TEST_DLLS}
        $<TARGET_FILE_DIR:tests_utilities>
      COMMENT "Copying Qt DLLs for tests_utilities"
    )
  endif()

  if(MINGW)
    get_filename_component(MINGW_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    set(MINGW_DLLS "")
    foreach(MINGW_DLL_NAME
            libgcc_s_seh-1.dll
            libstdc++-6.dll
            libwinpthread-1.dll)
      if(EXISTS "${MINGW_BIN_DIR}/${MINGW_DLL_NAME}")
        list(APPEND MINGW_DLLS "${MINGW_BIN_DIR}/${MINGW_DLL_NAME}")
      endif()
    endforeach()
    if(MINGW_DLLS)
      add_custom_command(TARGET tests_utilities POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${MINGW_DLLS}
          $<TARGET_FILE_DIR:tests_utilities>
        COMMENT "Copying MinGW runtime DLLs for tests_utilities"
      )
    endif()
  endif()
endif()

add_test(
  NAME ui_smoke
  COMMAND $<TARGET_FILE:TrenchKit> --smoke-test
)

set_tests_properties(ui_smoke PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=windows"
)
