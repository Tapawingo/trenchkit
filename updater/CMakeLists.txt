add_executable(updater
  main.cpp
)

set_target_properties(updater PROPERTIES
  AUTOMOC OFF
  AUTOUIC OFF
  AUTORCC OFF
)

target_compile_features(updater PRIVATE cxx_std_20)
