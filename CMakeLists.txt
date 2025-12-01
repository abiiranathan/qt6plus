cmake_minimum_required(VERSION 3.30)

project(qt6plus LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_EXAMPLES ON)



find_package(QT NAMES Qt6 REQUIRED 
  COMPONENTS 
  Core 
  Widgets 
  PrintSupport 
  Sql 
  Network 
  Gui 
  Multimedia 
  Bluetooth
)

find_package(Qt6 REQUIRED 
  COMPONENTS 
  Core
  Widgets
  PrintSupport
  Sql
  Network
  Gui
  Multimedia
  Bluetooth
)

set(HEADERS  
  include/Splitter.hpp
  include/GraphicsScene.hpp
  include/Delegates.hpp
  include/TableWidget.hpp
  include/DatabaseOptions.hpp  
  include/DatabaseConnection.hpp
  include/httpclient.hpp
  include/EnhancedTreeView.hpp
  include/BluetoothDevice.hpp
)

set(SOURCES 
  src/Splitter.cpp
  src/GraphicsScene.cpp
  src/TableWidget.cpp
  src/httpclient.cpp
  src/BluetoothDevice.cpp
  src/EnhancedTreeView.cpp
)

add_library(qt6plus SHARED ${HEADERS} ${SOURCES})

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


# Add bcrypt library
add_subdirectory(Bcrypt)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
