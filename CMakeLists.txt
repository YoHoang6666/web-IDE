cmake_minimum_required(VERSION 3.24)
project(webide VERSION 0.1.0 LANGUAGES CXX)

option(WEBIDE_ENABLE_DESKTOP_APP "Build the native Qt desktop application" ON)
option(WEBIDE_ENABLE_CEF "Enable Chromium Embedded Framework integration" OFF)
option(WEBIDE_USE_SCINTILLA "Enable the Scintilla editor backend" ON)
option(WEBIDE_USE_MONACO "Enable the Monaco webview bridge" OFF)
option(WEBIDE_BUILD_TESTS "Build test targets" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CompilerWarnings)
include(Sanitizers)
include(CEFConfig)
include(QtDeploy)

add_library(webide_core INTERFACE)
target_include_directories(webide_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(webide_core INTERFACE cxx_std_20)
if(WEBIDE_USE_SCINTILLA)
  target_compile_definitions(webide_core INTERFACE WEBIDE_USE_SCINTILLA=1)
endif()
if(WEBIDE_USE_MONACO)
  target_compile_definitions(webide_core INTERFACE WEBIDE_USE_MONACO=1)
endif()

if(WEBIDE_ENABLE_DESKTOP_APP)
  find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Network Sql Concurrent)

  if(COMMAND qt_standard_project_setup)
    qt_standard_project_setup()
  endif()

  add_library(webide_shell STATIC
    src/shell/MainWindow.cpp
    src/shell/MainWindow.h
    src/shell/DockLayoutManager.cpp
    src/shell/DockLayoutManager.h
    src/shell/StatusBarController.cpp
    src/shell/StatusBarController.h
    src/shell/MenuController.cpp
    src/shell/MenuController.h
  )
  target_link_libraries(webide_shell PUBLIC webide_core Qt6::Core Qt6::Gui Qt6::Widgets)
  webide_set_common_warnings(webide_shell)

  add_library(webide_workspace STATIC
    src/workspace/WorkspaceManager.cpp
    src/workspace/WorkspaceManager.h
    src/workspace/ProjectModel.cpp
    src/workspace/ProjectModel.h
    src/workspace/FileTreeModel.cpp
    src/workspace/FileTreeModel.h
    src/workspace/FileSystemService.cpp
    src/workspace/FileSystemService.h
    src/workspace/FileWatcherService.cpp
    src/workspace/FileWatcherService.h
  )
  target_link_libraries(webide_workspace PUBLIC webide_core Qt6::Core Qt6::Widgets)
  webide_set_common_warnings(webide_workspace)

  add_library(webide_editor STATIC
    src/editor/EditorHost.cpp
    src/editor/EditorHost.h
    src/editor/EditorTabManager.cpp
    src/editor/EditorTabManager.h
    src/editor/DocumentManager.cpp
    src/editor/DocumentManager.h
    src/editor/LanguageServiceBridge.cpp
    src/editor/LanguageServiceBridge.h
    src/editor/providers/ScintillaEditorWidget.cpp
    src/editor/providers/ScintillaEditorWidget.h
    src/editor/providers/MonacoWebViewBridge.cpp
    src/editor/providers/MonacoWebViewBridge.h
  )
  target_link_libraries(webide_editor PUBLIC webide_core webide_workspace Qt6::Core Qt6::Gui Qt6::Widgets)
  webide_set_common_warnings(webide_editor)

  add_library(webide_preview STATIC
    src/preview/PreviewPane.cpp
    src/preview/PreviewPane.h
    src/preview/PreviewSessionManager.cpp
    src/preview/PreviewSessionManager.h
    src/preview/LiveReloadController.cpp
    src/preview/LiveReloadController.h
    src/preview/CEFHostWidget.cpp
    src/preview/CEFHostWidget.h
    src/preview/DevToolsManager.cpp
    src/preview/DevToolsManager.h
  )
  target_link_libraries(webide_preview PUBLIC webide_core Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)
  webide_set_common_warnings(webide_preview)
  webide_configure_cef(webide_preview)

  add_library(webide_database STATIC
    src/database/DatabaseManager.cpp
    src/database/DatabaseManager.h
    src/database/SQLiteConnectionPool.cpp
    src/database/SQLiteConnectionPool.h
    src/database/QueryEngine.cpp
    src/database/QueryEngine.h
    src/database/TableModelAdapter.cpp
    src/database/TableModelAdapter.h
    src/database/MigrationService.cpp
    src/database/MigrationService.h
    src/database/DatabaseWatcherService.cpp
    src/database/DatabaseWatcherService.h
  )
  target_link_libraries(webide_database PUBLIC webide_core Qt6::Core Qt6::Sql Qt6::Widgets)
  webide_set_common_warnings(webide_database)

  add_library(webide_runtime STATIC
    src/runtime/LocalApiBridge.cpp
    src/runtime/LocalApiBridge.h
    src/runtime/ScriptExecutionContext.cpp
    src/runtime/ScriptExecutionContext.h
    src/runtime/SecurityPolicy.cpp
    src/runtime/SecurityPolicy.h
  )
  target_link_libraries(webide_runtime PUBLIC webide_core Qt6::Core)
  webide_set_common_warnings(webide_runtime)

  add_library(webide_sync STATIC
    src/sync/SyncCoordinator.cpp
    src/sync/SyncCoordinator.h
    src/sync/ChangeSet.h
    src/sync/ConflictResolver.cpp
    src/sync/ConflictResolver.h
  )
  target_link_libraries(webide_sync PUBLIC webide_core webide_workspace webide_database webide_preview Qt6::Core)
  webide_set_common_warnings(webide_sync)

  add_subdirectory(plugins)
  add_subdirectory(apps/desktop)
  
else()
  add_custom_target(webide_structure_check ALL
    COMMAND ${CMAKE_COMMAND} -E echo "web-IDE native structure generated without desktop dependencies"
  )
endif()

if(WEBIDE_BUILD_TESTS)
  enable_testing()
endif()
