qt_add_executable(TrenchKit
  main.cpp
  MainWindow.h
  MainWindow.cpp
  MainWindow.ui

  # Widgets
  widgets/TitleBar.h
  widgets/TitleBar.cpp
  widgets/InstallPathWidget.h
  widgets/InstallPathWidget.cpp

  # Utilities
  utils/FoxholeDetector.h
  utils/FoxholeDetector.cpp

  resources.qrc
)

target_link_libraries(TrenchKit
  PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

target_include_directories(TrenchKit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Post-build: Copy Qt and MinGW runtime DLLs for Windows
if(WIN32)
    # Qt DLLs
    add_custom_command(TARGET TrenchKit POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${Qt6_DIR}/../../../bin/Qt6Core.dll"
            "${Qt6_DIR}/../../../bin/Qt6Gui.dll"
            "${Qt6_DIR}/../../../bin/Qt6Widgets.dll"
            "${Qt6_DIR}/../../../bin/Qt6Network.dll"
            "${Qt6_DIR}/../../../bin/Qt6Concurrent.dll"
            $<TARGET_FILE_DIR:TrenchKit>
        COMMENT "Copying Qt DLLs to output directory"
    )

    # MinGW runtime DLLs
    if(MINGW)
        get_filename_component(MINGW_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
        add_custom_command(TARGET TrenchKit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                "${MINGW_BIN_DIR}/libstdc++-6.dll"
                "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                $<TARGET_FILE_DIR:TrenchKit>
            COMMENT "Copying MinGW runtime DLLs to output directory"
        )
    endif()

    # Qt platform plugin
    add_custom_command(TARGET TrenchKit POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:TrenchKit>/platforms
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${Qt6_DIR}/../../../plugins/platforms/qwindows.dll"
            $<TARGET_FILE_DIR:TrenchKit>/platforms/
        COMMENT "Copying Qt platform plugin to output directory"
    )
endif()
