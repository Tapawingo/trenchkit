qt_add_executable(FoxholeModManager
  main.cpp
  MainWindow.h
  MainWindow.cpp
  MainWindow.ui
  resources.qrc
)

target_link_libraries(FoxholeModManager
  PRIVATE
    Qt6::Widgets
    Qt6::Network
)

target_include_directories(FoxholeModManager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
