cmake_minimum_required(VERSION 3.30)

project(qt6plus LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_EXAMPLES ON)

# Add bcrypt library
add_subdirectory(Bcrypt)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport Sql Network Gui Multimedia Bluetooth)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport Sql Network Gui Multimedia Bluetooth)

add_library(qt6plus SHARED
  Splitter.hpp
  Splitter.cpp

  GraphicsScene.hpp
  GraphicsScene.cpp

  Delegates.hpp
  TableWidget.hpp
  TableWidget.cpp
  
  DatabaseOptions.hpp  
  DatabaseConnection.hpp

  httpclient.hpp
  httpclient.cpp

  EnhancedTreeView.hpp
  EnhancedTreeView.cpp

  BluetoothDevice.hpp
  BluetoothDevice.cpp
)

target_link_libraries(qt6plus PUBLIC
    Qt6::Widgets
    Qt6::Core
    Qt6::PrintSupport
    Qt6::Sql
    Qt6::Network
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Bluetooth
)

target_compile_definitions(qt6plus PRIVATE SPLITTER_LIBRARY)


if(BUILD_EXAMPLES)
    add_executable(main main.cpp)
    target_link_libraries(main PRIVATE qt6plus bcrypt)

    add_executable(treeview treeview.cpp)
    target_link_libraries(treeview PRIVATE qt6plus)

    add_executable(mediaPlayer mediaPlayer.cpp)
    target_link_libraries(mediaPlayer PRIVATE qt6plus)

endif()
