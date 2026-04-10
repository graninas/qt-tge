cmake_minimum_required(VERSION 3.5)
project(QtTGE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt5 Widgets
set(CMAKE_PREFIX_PATH "/usr/lib/qt5")
find_package(Qt5 REQUIRED COMPONENTS Widgets Core)

# UI file
set(UI_FILES src/ui/mainwindow.ui)

# Sources
set(SOURCES src/main.cpp src/mainwindow.cpp src/graphwidget.cpp)
set(HEADERS src/mainwindow.h src/graphwidget.h)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(Qt-TGE ${SOURCES})
target_link_libraries(Qt-TGE Qt5::Widgets Qt5::Core)

# For IDEs
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Qt-TGE)
# Add UI files to the executable (AUTOUIC handles .ui files automatically)
target_sources(Qt-TGE PRIVATE ${UI_FILES})
target_include_directories(Qt-TGE PRIVATE src)

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
