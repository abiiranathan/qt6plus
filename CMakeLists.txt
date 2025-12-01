cmake_minimum_required(VERSION 3.30)

project(qt6plus 
    VERSION 1.0.0
    DESCRIPTION "Qt6 Plus - Extended Qt6 Widgets Library"
    LANGUAGES CXX
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Include GNUInstallDirs for standard installation directories
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

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

# Create the library
add_library(qt6plus ${HEADERS} ${SOURCES})

# Add alias for consistent usage
add_library(qt6plus::qt6plus ALIAS qt6plus)

# Set library properties
set_target_properties(qt6plus PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${HEADERS}"
    EXPORT_NAME qt6plus
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

# Include directories
target_include_directories(qt6plus
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Add bcrypt library
add_subdirectory(Bcrypt)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation rules
install(TARGETS qt6plus
    EXPORT qt6plusTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Runtime
        NAMELINK_COMPONENT Development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Development
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Runtime
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qt6plus
        COMPONENT Development
)

# Install export targets
install(EXPORT qt6plusTargets
    FILE qt6plusTargets.cmake
    NAMESPACE qt6plus::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qt6plus
    COMPONENT Development
)

# Create and install package config files
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/qt6plusConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/qt6plusConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qt6plus
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/qt6plusConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/qt6plusConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/qt6plusConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qt6plus
    COMPONENT Development
)

# Generate pkg-config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/qt6plus.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/qt6plus.pc
    @ONLY
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/qt6plus.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    COMPONENT Development
)

# Print installation summary
message(STATUS "")
message(STATUS "qt6plus ${PROJECT_VERSION} Configuration Summary")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Library dir:    ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "  Include dir:    ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "  Build examples: ${BUILD_EXAMPLES}")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "")