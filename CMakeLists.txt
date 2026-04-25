cmake_minimum_required(VERSION 3.11)
project(QtTGE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FetchContent is used to download header-only libraries at configure time
include(FetchContent)
FetchContent_Declare(
    cpp_parsec_free
    GIT_REPOSITORY https://github.com/graninas/cpp_parsec_free.git
    GIT_TAG master
)
FetchContent_MakeAvailable(cpp_parsec_free)

# Find Qt5 Widgets, Core, and Gui
set(CMAKE_PREFIX_PATH "/usr/lib/qt5")
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)

# UI file
set(UI_FILES src/ui/mainwindow.ui)

# Sources
set(SOURCES src/main.cpp src/ui/mainwindow.cpp src/ui/graphwidget.cpp src/ui/graphwidget_edges.cpp src/ui/graphwidget_locations.cpp src/ui/gui_model.cpp src/ui/graphwidget_helpers.cpp src/ui/locationdialog.cpp src/ui/edgedialog.cpp src/ui/edgelistitemwidget.cpp src/ui/graphwidget_errors.cpp)
set(HEADERS src/ui/mainwindow.h src/ui/graphwidget.h src/ui/edgedialog.h src/ui/edgelistitemwidget.h)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(Qt-TGE ${SOURCES})
target_link_libraries(Qt-TGE PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui)

# Build formula parser implementation as a static library so tests can link it
add_library(tge_formula STATIC src/tge/formula/parser.cpp src/tge/formula/translator.cpp)
target_include_directories(tge_formula PUBLIC src ${cpp_parsec_free_SOURCE_DIR} ${cpp_parsec_free_SOURCE_DIR}/cpp_free_parsec)

# Link parser library into main executable
if(TARGET Qt-TGE)
  target_link_libraries(Qt-TGE PRIVATE tge_formula)
endif()

# Link parser library into formula test
if(TARGET QtTGE_test_formula_translation)
  target_link_libraries(QtTGE_test_formula_translation PRIVATE tge_formula)
endif()

# For IDEs
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Qt-TGE)
# Add UI files to the executable (AUTOUIC handles .ui files automatically)
target_sources(Qt-TGE PRIVATE ${UI_FILES})
# Make project headers available to the executable
# Also add the fetched cpp_parsec_free source dir so the header-only library can be included
target_include_directories(Qt-TGE PRIVATE src ${cpp_parsec_free_SOURCE_DIR} ${cpp_parsec_free_SOURCE_DIR}/cpp_free_parsec)

# Add Qt5 include directories explicitly for header resolution
include_directories(
    /usr/include/x86_64-linux-gnu/qt5
    /usr/include/x86_64-linux-gnu/qt5/QtWidgets
    /usr/include/x86_64-linux-gnu/qt5/QtGui
    /usr/include/x86_64-linux-gnu/qt5/QtCore
)

# Optionally, group sources for IDEs
source_group("UI Files" FILES ${UI_FILES})
source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

# Enable testing
include(CTest)
enable_testing()

# Add test source
add_executable(QtTGE_tests tests/test_dummy.cpp)
target_link_libraries(QtTGE_tests PRIVATE Qt5::Core)
add_test(NAME DummyTest COMMAND ${CMAKE_BINARY_DIR}/QtTGE_tests)

add_executable(QtTGE_test_graph_structure tests/test_graph_structure.cpp)
target_link_libraries(QtTGE_test_graph_structure PRIVATE Qt5::Core)
add_test(NAME GraphStructureTest COMMAND ${CMAKE_BINARY_DIR}/QtTGE_test_graph_structure)

add_executable(QtTGE_test_editor_dynamic_graph tests/test_editor_dynamic_graph.cpp)
target_link_libraries(QtTGE_test_editor_dynamic_graph PRIVATE Qt5::Core)
add_test(NAME EditorDynamicGraphTest COMMAND ${CMAKE_BINARY_DIR}/QtTGE_test_editor_dynamic_graph)

add_executable(QtTGE_test_editor_loop_edges tests/test_editor_loop_edges.cpp)
target_link_libraries(QtTGE_test_editor_loop_edges PRIVATE Qt5::Core)
add_test(NAME EditorLoopEdgesTest COMMAND ${CMAKE_BINARY_DIR}/QtTGE_test_editor_loop_edges)

add_executable(QtTGE_test_player_runtime tests/test_player_runtime.cpp)
target_link_libraries(QtTGE_test_player_runtime PRIVATE Qt5::Core)
add_test(NAME PlayerRuntimeTest COMMAND ${CMAKE_BINARY_DIR}/QtTGE_test_player_runtime)

add_executable(QtTGE_test_player_intermediate_path tests/test_player_intermediate_path.cpp)
target_link_libraries(QtTGE_test_player_intermediate_path PRIVATE Qt5::Core)
add_test(NAME PlayerIntermediatePathTest COMMAND ${CMAKE_BINARY_DIR}/QtTGE_test_player_intermediate_path)

add_executable(QtTGE_test_formula_translation tests/test_formula_translation.cpp)
target_link_libraries(QtTGE_test_formula_translation PRIVATE Qt5::Core)
target_link_libraries(QtTGE_test_formula_translation PRIVATE tge_formula)
add_test(NAME FormulaTranslationTest COMMAND ${CMAKE_BINARY_DIR}/QtTGE_test_formula_translation)
if(TARGET QtTGE_test_formula_translation)
  target_include_directories(QtTGE_test_formula_translation PRIVATE ${cpp_parsec_free_SOURCE_DIR} ${cpp_parsec_free_SOURCE_DIR}/cpp_free_parsec src)
endif()

# Expose cpp_parsec_free headers to the tests (if targets exist)
foreach(tgt QtTGE_tests QtTGE_test_graph_structure QtTGE_test_editor_dynamic_graph QtTGE_test_editor_loop_edges QtTGE_test_player_runtime QtTGE_test_player_intermediate_path QtTGE_test_formula_translation)
  if(TARGET ${tgt})
    target_include_directories(${tgt} PRIVATE ${cpp_parsec_free_SOURCE_DIR} ${cpp_parsec_free_SOURCE_DIR}/cpp_free_parsec)
  endif()
endforeach()
