cmake_minimum_required(VERSION 3.16)

project(test_Widgets VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets) # Load QT Packages
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)     # Execution System
    qt_add_executable(test_Widgets
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        OcctViewerWidget.h
        OcctViewerWidget.cpp
        .gitignore
    )
else()
    add_executable(test_Widgets ${PROJECT_SOURCES})
endif()

# OpenCascade via vcpkg
find_package(OpenCASCADE CONFIG REQUIRED)
target_include_directories(test_Widgets PRIVATE ${OpenCASCADE_INCLUDE_DIR}) # Load OCCT dependencies
target_link_libraries(test_Widgets PRIVATE ${OpenCASCADE_LIBRARIES})

# ---- Qt ----
target_link_libraries(test_Widgets PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(test_Widgets)
endif()

