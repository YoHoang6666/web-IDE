add_executable(webide_desktop
  main.cpp
  AppBootstrap.cpp
  AppBootstrap.h
  resources/app.qrc
)

webide_deploy_qt(webide_desktop)

target_include_directories(webide_desktop PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(webide_desktop PRIVATE
  webide_shell
  webide_workspace
  webide_editor
  webide_preview
  webide_database
  webide_runtime
  webide_sync
  webide_plugin_sdk
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  Qt6::Network
  Qt6::Sql
  Qt6::Concurrent
)

set_target_properties(webide_desktop PROPERTIES
  WIN32_EXECUTABLE TRUE
  OUTPUT_NAME "web-ide"
)

webide_enable_sanitizers(webide_desktop)
webide_set_common_warnings(webide_desktop)
