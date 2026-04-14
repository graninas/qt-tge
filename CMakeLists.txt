cmake_minimum_required(VERSION 3.5)
project(QtTGE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt5 Widgets, Core, and Gui
set(CMAKE_PREFIX_PATH "/usr/lib/qt5")
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)

# UI file
set(UI_FILES src/ui/mainwindow.ui)

# Sources
set(SOURCES src/main.cpp src/mainwindow.cpp src/graphwidget.cpp src/graphwidget_edges.cpp src/graphwidget_locations.cpp src/gui_model.cpp src/graphwidget_helpers.cpp src/locationdialog.cpp src/edgedialog.cpp src/edgelistitemwidget.cpp src/graphwidget_errors.cpp)
set(HEADERS src/mainwindow.h src/graphwidget.h src/edgedialog.h src/edgelistitemwidget.h)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(Qt-TGE ${SOURCES})
target_link_libraries(Qt-TGE Qt5::Widgets Qt5::Core Qt5::Gui)

# For IDEs
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Qt-TGE)
# Add UI files to the executable (AUTOUIC handles .ui files automatically)
target_sources(Qt-TGE PRIVATE ${UI_FILES})
target_include_directories(Qt-TGE PRIVATE src)

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
target_link_libraries(QtTGE_tests Qt5::Core)
add_test(NAME DummyTest COMMAND ${CMAKE_BINARY_DIR}/QtTGE_tests)

add_executable(QtTGE_test_graph_structure tests/test_graph_structure.cpp)
target_link_libraries(QtTGE_test_graph_structure Qt5::Core)
add_test(NAME GraphStructureTest COMMAND ${CMAKE_BINARY_DIR}/QtTGE_test_graph_structure)

add_executable(QtTGE_test_editor_dynamic_graph tests/test_editor_dynamic_graph.cpp)
target_link_libraries(QtTGE_test_editor_dynamic_graph Qt5::Core)
add_test(NAME EditorDynamicGraphTest COMMAND ${CMAKE_BINARY_DIR}/QtTGE_test_editor_dynamic_graph)

add_executable(QtTGE_test_player_runtime tests/test_player_runtime.cpp)
target_link_libraries(QtTGE_test_player_runtime Qt5::Core)
add_test(NAME PlayerRuntimeTest COMMAND ${CMAKE_BINARY_DIR}/QtTGE_test_player_runtime)

add_executable(QtTGE_test_player_intermediate_path tests/test_player_intermediate_path.cpp)
target_link_libraries(QtTGE_test_player_intermediate_path Qt5::Core)
add_test(NAME PlayerIntermediatePathTest COMMAND ${CMAKE_BINARY_DIR}/QtTGE_test_player_intermediate_path)
