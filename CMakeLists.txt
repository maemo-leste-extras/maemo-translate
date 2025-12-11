cmake_minimum_required(VERSION 3.16)

project(maemo-translate
        VERSION 0.3.5
        DESCRIPTION "Library for language translation without using the cloud."
        HOMEPAGE_URL https://github.com/maemo-leste-extras/maemo-translate
        LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)
set(MAEMO_TRANSLATE_DESKTOP_DIR /usr/share/applications/hildon)

include(GNUInstallDirs)  # initializes ${CMAKE_INSTALL_*DIR} to sane defaults
include(CMakePackageConfigHelpers)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(kotki REQUIRED)
find_package(CLI11 REQUIRED)
find_package(qt6maemo REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CPUINFO REQUIRED IMPORTED_TARGET libcpuinfo)

# custom imported target for cpuinfo because we are not using CMake
# https://git.maemo.org/leste/bugtracker/issues/819
add_library(cpuinfo::cpuinfo INTERFACE IMPORTED)
target_link_libraries(cpuinfo::cpuinfo INTERFACE PkgConfig::CPUINFO)
target_include_directories(cpuinfo::cpuinfo INTERFACE ${CPUINFO_INCLUDE_DIRS})

# Tell CMake to create the maemo-translate executable
add_executable(maemo-translate
        src/main.cpp
        src/ctx.h
        src/ctx.cpp
        src/lib/debounce.h
        src/lib/blocking_queue.h
        src/lib/blocking_queue.cpp
        src/lib/config.cpp
        src/lib/config.h
        src/lib/utils.cpp
        src/lib/utils.h
        src/lib/textedit.cpp
        src/lib/textedit.h
        src/about.cpp
        src/about.h
        src/about.ui
        src/translateinwidget.cpp
        src/translateinwidget.h
        src/translateinwidget.ui
        src/translateoutwidget.cpp
        src/translateoutwidget.h
        src/translateoutwidget.ui
        src/mainwindow.cpp
        src/mainwindow.ui
        src/resources.qrc)

target_link_libraries(maemo-translate
        Qt6::Widgets
        CLI11::CLI11
        qt6maemo::qt6maemo
        kotki::kotki-lib-SHARED)
target_include_directories(maemo-translate PRIVATE src)

if(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm*")
  target_compile_definitions(maemo-translate PRIVATE ARM=1)
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
  target_compile_definitions(maemo-translate PRIVATE DEBUG=1)
endif()

set_target_properties(maemo-translate PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )

set(EXECUTABLE_FLAG)

install(TARGETS maemo-translate
        DESTINATION bin
        )

add_subdirectory(desktop)
