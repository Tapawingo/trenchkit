qt_add_executable(TrenchKit
  main.cpp
  MainWindow.h
  MainWindow.cpp
  MainWindow.ui

  # Widgets
  widgets/TitleBar.h
  widgets/TitleBar.cpp
  widgets/GradientFrame.h
  widgets/GradientFrame.cpp
  widgets/PanelFrame.h
  widgets/PanelFrame.cpp
  widgets/InstallPathWidget.h
  widgets/InstallPathWidget.cpp
  widgets/ModListWidget.h
  widgets/ModListWidget.cpp
  widgets/DraggableModList.h
  widgets/DraggableModList.cpp
  widgets/ModRowWidget.h
  widgets/ModRowWidget.cpp
  widgets/ProfileRowWidget.h
  widgets/ProfileRowWidget.cpp
  widgets/RightPanelWidget.h
  widgets/RightPanelWidget.cpp
  widgets/ActionsWidget.h
  widgets/ActionsWidget.cpp
  widgets/BackupWidget.h
  widgets/BackupWidget.cpp
  widgets/LogEntryWidget.h
  widgets/LogEntryWidget.cpp
  widgets/ActivityLogWidget.h
  widgets/ActivityLogWidget.cpp
  widgets/LaunchWidget.h
  widgets/LaunchWidget.cpp
  widgets/ModMetadataDialog.h
  widgets/ModMetadataDialog.cpp
  widgets/ProfileManagerWidget.h
  widgets/ProfileManagerWidget.cpp
  widgets/SettingsWidget.h
  widgets/SettingsWidget.cpp
  widgets/FileSelectionDialog.h
  widgets/FileSelectionDialog.cpp
  widgets/AddModDialog.h
  widgets/AddModDialog.cpp

  # Utilities
  utils/FoxholeDetector.h
  utils/FoxholeDetector.cpp
  utils/ModInfo.h
  utils/ModInfo.cpp
  utils/ModManager.h
  utils/ModManager.cpp
  utils/ProfileInfo.h
  utils/ProfileInfo.cpp
  utils/ProfileManager.h
  utils/ProfileManager.cpp
  utils/Theme.h
  utils/Theme.cpp
  utils/ArchiveExtractor.h
  utils/ArchiveExtractor.cpp
  utils/UpdaterService.h
  utils/UpdaterService.cpp
  utils/UpdateArchiveExtractor.h
  utils/UpdateArchiveExtractor.cpp
  utils/UpdateCleanup.h
  utils/UpdateCleanup.cpp

  resources.qrc
  app.rc
)

target_compile_definitions(TrenchKit PRIVATE
  TRENCHKIT_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(TrenchKit
  PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
    zip
)

target_include_directories(TrenchKit
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Post-build: Copy Qt and MinGW runtime DLLs for Windows
if(WIN32)
    add_dependencies(TrenchKit updater)
    add_custom_command(TARGET TrenchKit POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:updater>
            $<TARGET_FILE_DIR:TrenchKit>
        COMMENT "Copying updater helper to output directory"
    )

    set(QT_TLS_PLUGIN_DIR "${Qt6_DIR}/../../../plugins/tls")
    if(EXISTS "${QT_TLS_PLUGIN_DIR}/qschannelbackend.dll")
        add_custom_command(TARGET TrenchKit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                $<TARGET_FILE_DIR:TrenchKit>/tls
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_TLS_PLUGIN_DIR}/qschannelbackend.dll"
                $<TARGET_FILE_DIR:TrenchKit>/tls/
            COMMENT "Copying Qt TLS backend plugin to output directory"
        )
    endif()

    # Qt DLLs
    set(QT_BIN_DIR "${Qt6_DIR}/../../../bin")
    set(QT_APP_DLLS "")
    foreach(QT_DLL_NAME
            Qt6Core.dll
            Qt6Gui.dll
            Qt6Widgets.dll
            Qt6Network.dll
            Qt6Concurrent.dll)
        if(EXISTS "${QT_BIN_DIR}/${QT_DLL_NAME}")
            list(APPEND QT_APP_DLLS "${QT_BIN_DIR}/${QT_DLL_NAME}")
        endif()
    endforeach()
    if(QT_APP_DLLS)
        add_custom_command(TARGET TrenchKit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${QT_APP_DLLS}
                $<TARGET_FILE_DIR:TrenchKit>
            COMMENT "Copying Qt DLLs to output directory"
        )
    endif()

    # MinGW runtime DLLs
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
            add_custom_command(TARGET TrenchKit POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${MINGW_DLLS}
                    $<TARGET_FILE_DIR:TrenchKit>
                COMMENT "Copying MinGW runtime DLLs to output directory"
            )
        endif()
    endif()

    # Qt platform plugin
    set(QT_PLATFORM_DLL "${Qt6_DIR}/../../../plugins/platforms/qwindows.dll")
    if(EXISTS "${QT_PLATFORM_DLL}")
        add_custom_command(TARGET TrenchKit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                $<TARGET_FILE_DIR:TrenchKit>/platforms
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_PLATFORM_DLL}"
                $<TARGET_FILE_DIR:TrenchKit>/platforms/
            COMMENT "Copying Qt platform plugin to output directory"
        )
    endif()
endif()
