cmake_minimum_required(VERSION 3.16)

project(GUI_MAPS VERSION 1.0.0 LANGUAGES CXX)

set(APP_DISPLAY_NAME "MAPS0 Desktop Client")

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
elseif(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets OpenGL Charts)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    viewerwidget.cpp
    viewerwidget.h
    iges_parsing.h
    nurbs.h
    theme.h
    nurbs_evaluation.h
    surfacecontroldialog.h
    surfacecontroldialog.cpp
    principaldatadialog.h
    principaldatadialog.cpp
    wavesettingsdialog.h
    wavesettingsdialog.cpp
    gauss_points.h
    surfaceworker.h
    surfaceworker.cpp
    projectassembler.cpp
    projectassembler.h
    runconfigdialog.cpp
    runconfigdialog.h
    resultsviewer.cpp
    resultsviewer.h
    resources/resources.qrc
)

qt_add_executable(GUI_MAPS
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(GUI_MAPS PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::OpenGL
    Qt6::Charts
)

set_target_properties(GUI_MAPS PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    OUTPUT_NAME "$<IF:$<PLATFORM_ID:Darwin>,${APP_DISPLAY_NAME},GUI_MAPS>"
    MACOSX_BUNDLE_BUNDLE_NAME       "${APP_DISPLAY_NAME}"
    MACOSX_BUNDLE_GUI_IDENTIFIER    "com.maps0.desktopclient"
    MACOSX_BUNDLE_BUNDLE_VERSION    "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

qt_finalize_executable(GUI_MAPS)

# ── Install rules ─────────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS GUI_MAPS
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Bundle Qt runtime alongside the binary so the install dir is self-contained.
# Run after install:    cmake --install build --prefix dist/
if(WIN32)
    # Locate windeployqt next to the Qt6 build tools
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT_EXECUTABLE)
        install(CODE "
            execute_process(COMMAND \"${WINDEPLOYQT_EXECUTABLE}\"
                --release --no-translations --no-system-d3d-compiler
                --no-compiler-runtime
                \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/GUI_MAPS.exe\")
        ")
    endif()
elseif(APPLE)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(MACDEPLOYQT_EXECUTABLE)
        install(CODE "
            execute_process(COMMAND \"${MACDEPLOYQT_EXECUTABLE}\"
                \"\${CMAKE_INSTALL_PREFIX}/${APP_DISPLAY_NAME}.app\")
        ")
    endif()
endif()

# CPack — generates installers via `cpack -G <gen>` after install
set(CPACK_PACKAGE_NAME           "MAPS0DesktopClient")
set(CPACK_PACKAGE_VENDOR         "MAPS0")
set(CPACK_PACKAGE_VERSION        "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MAPS0 marine hydrodynamic solver desktop client")
set(CPACK_PACKAGE_INSTALL_DIRECTORY   "${APP_DISPLAY_NAME}")
if(WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ")
endif()
include(CPack)
