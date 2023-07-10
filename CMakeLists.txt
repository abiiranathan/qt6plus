cmake_minimum_required(VERSION 3.14)

project(qt6plus LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_EXAMPLES ON)

# Add bcrypt library
add_subdirectory(Bcrypt)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
        Core
        Widgets
        PrintSupport
        Sql
        Network
        Gui
        Multimedia
        Bluetooth
 )

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Core
    Widgets
    PrintSupport
    Sql
    Network
    Gui
    Multimedia
    Bluetooth
)

add_library(qt6plus SHARED
  Splitter.h
  Splitter.cpp

  GraphicsScene.h
  GraphicsScene.cpp

  Delegates.h
  TableWidget.h
  TableWidget.cpp

  errormanager.h
  errormanager.cpp
  DatabaseUtils.h
  DatabaseUtils.cpp

  httpclient.h
  httpclient.cpp

  EnhancedTreeView.h
  EnhancedTreeView.cpp

  BluetoothDevice.h
  BluetoothDevice.cpp
)

target_link_libraries(qt6plus PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Bluetooth
)

target_compile_definitions(qt6plus PRIVATE SPLITTER_LIBRARY)


if(BUILD_EXAMPLES)
    add_executable(main main.cpp)
    target_link_libraries(main PRIVATE qt6plus bcrypt)

    add_executable(db db.cpp)
    target_link_libraries(db PRIVATE qt6plus)

    add_executable(treeview treeview.cpp)
    target_link_libraries(treeview PRIVATE qt6plus)

    add_executable(mediaPlayer mediaPlayer.cpp)
    target_link_libraries(mediaPlayer PRIVATE qt6plus)

endif()
