cmake_minimum_required(VERSION 3.25)

project(MalloyWriter VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Concurrent Test)

add_library(malloywriter_core STATIC
    src/assistant/AssistantService.cpp
    src/base/Color.cpp
    src/base/Log.cpp
    src/base/PathUtils.cpp
    src/base/Theme.cpp
    src/platform/BuildSystem.cpp
    src/platform/BuildService.cpp
    src/platform/CMakeFileApi.cpp
    src/platform/CommandRegistry.cpp
    src/platform/DocumentService.cpp
    src/platform/ProcessRunner.cpp
    src/platform/ProjectService.cpp
    src/platform/SearchService.cpp
    src/platform/SettingsService.cpp
    src/platform/ToolchainKit.cpp
    src/platform/WorkspaceModel.cpp
    src/editor/Breadcrumbs.cpp
    src/editor/CodeEditor.cpp
    src/editor/CppSyntaxHighlighter.cpp
    src/editor/Document.cpp
    src/editor/EditorArea.cpp
    src/editor/EditorTabBar.cpp
    src/editor/EditorWidget.cpp
    src/editor/Minimap.cpp
    src/languages/CppLanguageSupport.cpp
    src/languages/ClangdLanguageService.cpp
    src/languages/JsonRpcTransport.cpp
    src/languages/LanguageClient.cpp
    src/languages/LspDiagnosticMapper.cpp
    src/debug/DebugAdapterClient.cpp
    src/workbench/ActivityBar.cpp
    src/workbench/BottomPanel.cpp
    src/workbench/CommandPalette.cpp
    src/workbench/ComposerDialog.cpp
    src/workbench/Icon.cpp
    src/workbench/MainWindow.cpp
    src/workbench/OpenEditorsList.cpp
    src/workbench/OutputPanel.cpp
    src/workbench/ProjectExplorer.cpp
    src/workbench/RunDebugView.cpp
    src/workbench/SearchView.cpp
    src/workbench/SettingsView.cpp
    src/workbench/Sidebar.cpp
    src/workbench/StaticSidebarViews.cpp
    src/workbench/StatusBar.cpp
    src/workbench/ThemeApplication.cpp
    src/workbench/WelcomeScreen.cpp
)

target_include_directories(malloywriter_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(malloywriter_core
    PUBLIC
        Qt6::Widgets
        Qt6::Svg
        Qt6::Concurrent
)

if(MSVC)
    target_compile_options(malloywriter_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(malloywriter_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(MalloyWriter
    src/main.cpp
)

target_link_libraries(MalloyWriter PRIVATE malloywriter_core)

enable_testing()

function(add_malloywriter_test test_name source_file)
    add_executable(${test_name} ${source_file})
    target_link_libraries(${test_name} PRIVATE malloywriter_core Qt6::Test)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_malloywriter_test(command_registry_tests tests/command_registry_tests.cpp)
add_malloywriter_test(theme_tests tests/theme_tests.cpp)
add_malloywriter_test(lsp_diagnostic_mapper_tests tests/lsp_diagnostic_mapper_tests.cpp)
add_malloywriter_test(project_service_tests tests/project_service_tests.cpp)
add_malloywriter_test(cpp_syntax_highlighter_tests tests/cpp_syntax_highlighter_tests.cpp)
add_malloywriter_test(search_service_tests tests/search_service_tests.cpp)
add_malloywriter_test(document_tests tests/document_tests.cpp)
add_malloywriter_test(document_service_tests tests/document_service_tests.cpp)
add_malloywriter_test(workspace_model_tests tests/workspace_model_tests.cpp)
add_malloywriter_test(build_system_tests tests/build_system_tests.cpp)
add_malloywriter_test(build_service_tests tests/build_service_tests.cpp)
add_malloywriter_test(cmake_file_api_tests tests/cmake_file_api_tests.cpp)
add_malloywriter_test(jsonrpc_transport_tests tests/jsonrpc_transport_tests.cpp)
add_malloywriter_test(toolchain_kit_tests tests/toolchain_kit_tests.cpp)
