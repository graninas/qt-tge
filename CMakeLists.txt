cmake_minimum_required(VERSION 3.5)
project(QtTGE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt5 Widgets
set(CMAKE_PREFIX_PATH "/usr/lib/qt5")
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# UI file
set(UI_FILES ui/mainwindow.ui)

# Sources
set(SOURCES src/main.cpp src/mainwindow.cpp src/graphwidget.cpp)
set(HEADERS src/mainwindow.h src/graphwidget.h)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(QtWidgetsApp ${SOURCES})
target_link_libraries(QtWidgetsApp Qt5::Widgets)

# For IDEs
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT QtWidgetsApp)
