cmake_minimum_required(VERSION 3.14)

project(splitter LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_EXAMPLES OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

add_library(splitter SHARED
  splitter_global.h
  splitter.h
  GraphicsScene.h
)

target_link_libraries(splitter PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core)
target_compile_definitions(splitter PRIVATE SPLITTER_LIBRARY)


if(BUILD_EXAMPLES)
    add_executable(main main.cpp)
    target_link_libraries(main PRIVATE splitter)
    target_link_libraries(main PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core)
endif()
